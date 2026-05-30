#include "StrategyEngine.hpp"

#include "JsonUtils.hpp"

#include "ZScoreStrategy.hpp"
#include "ExecutionStrengthStrategy.hpp"
#include "OrderbookImbalanceStrategy.hpp"
#include "TickMAStrategy.hpp"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <stdexcept>

SignalResult StrategyEngine::handleEvent(const json& body) {
    cleanupExpiredStates();

    std::string symbol =
        get_string_or_default(body, "symbol", "UNKNOWN");

    if (symbol == "UNKNOWN") {
        throw std::runtime_error("symbol is required");
    }

    std::string event_type =
        get_string_or_default(body, "event_type", "UNKNOWN");

    // POSITION 이벤트는 보유 상태만 갱신하고 HOLD 반환
    if (event_type == "POSITION") {
        updatePositionState(body);

        return {
            "RISK_POSITION_UPDATE",
            "HOLD",
            0.0,
            1.0,
            "Position state updated"
        };
    }

    SymbolState& state = states[symbol];
    state.symbol = symbol;
    state.last_update = std::chrono::steady_clock::now();

    updateState(state, body);

    std::vector<SignalResult> results =
        runAvailableStrategies(state);

    SignalResult combined =
        combineResults(results);

    SignalResult risk_checked =
        risk_manager.applyRiskRules(symbol, combined, state);

    return risk_checked;
}

size_t StrategyEngine::getStateCount() const {
    return states.size();
}

void StrategyEngine::cleanupExpiredStates() {
    auto now = std::chrono::steady_clock::now();

    for (auto it = states.begin(); it != states.end();) {
        auto elapsed = now - it->second.last_update;

        if (elapsed > STATE_TIMEOUT) {
            std::cout << "[Cleanup] Removed inactive symbol: "
                      << it->first << std::endl;

            it = states.erase(it);
        }
        else {
            ++it;
        }
    }
}

void StrategyEngine::updateState(SymbolState& state, const json& body) {
    std::string event_type =
        get_string_or_default(body, "event_type", "UNKNOWN");

    if (event_type == "TRADE") {
        updateTradeState(state, body);
    }
    else if (event_type == "ORDERBOOK") {
        updateOrderbookState(state, body);
    }
    else {
        updateFlexibleState(state, body);
    }
}

void StrategyEngine::updateTradeState(SymbolState& state, const json& body) {
    if (body.contains("price") && !body["price"].is_null()) {
        double price = body["price"].get<double>();

        if (price > 0.0) {
            addTickPrice(state, price);
        }
    }

    if (body.contains("execution_strength") &&
        !body["execution_strength"].is_null()) {
        state.execution_strength =
            body["execution_strength"].get<double>();
        state.has_execution_data = true;
    }

    if (body.contains("price_change_rate") &&
        !body["price_change_rate"].is_null()) {
        state.price_change_rate =
            body["price_change_rate"].get<double>();
    }

    if (body.contains("volume_change_rate") &&
        !body["volume_change_rate"].is_null()) {
        state.volume_change_rate =
            body["volume_change_rate"].get<double>();
    }
}

void StrategyEngine::updateOrderbookState(SymbolState& state, const json& body) {
    if (body.contains("bid_prices") && body["bid_prices"].is_array()) {
        state.bid_prices =
            body["bid_prices"].get<std::vector<double>>();
    }

    if (body.contains("ask_prices") && body["ask_prices"].is_array()) {
        state.ask_prices =
            body["ask_prices"].get<std::vector<double>>();
    }

    if (body.contains("bid_volumes") && body["bid_volumes"].is_array()) {
        state.bid_volumes =
            body["bid_volumes"].get<std::vector<double>>();
    }

    if (body.contains("ask_volumes") && body["ask_volumes"].is_array()) {
        state.ask_volumes =
            body["ask_volumes"].get<std::vector<double>>();
    }

    if (!state.bid_volumes.empty() && !state.ask_volumes.empty()) {
        state.total_bid_volume = sumVector(state.bid_volumes);
        state.total_ask_volume = sumVector(state.ask_volumes);
        state.has_orderbook_data = true;
    }

    if (body.contains("price_change_rate") &&
        !body["price_change_rate"].is_null()) {
        state.price_change_rate =
            body["price_change_rate"].get<double>();
    }
}

void StrategyEngine::updateFlexibleState(SymbolState& state, const json& body) {
    if (body.contains("price")) {
        updateTradeState(state, body);
    }

    if (body.contains("bid_volumes") || body.contains("ask_volumes") ||
        body.contains("bid_prices") || body.contains("ask_prices")) {
        updateOrderbookState(state, body);
    }

    if (body.contains("tick_prices") && body["tick_prices"].is_array()) {
        std::vector<double> prices =
            body["tick_prices"].get<std::vector<double>>();

        for (double price : prices) {
            if (price > 0.0) {
                addTickPrice(state, price);
            }
        }
    }
}

void StrategyEngine::updatePositionState(const json& body) {
    std::string symbol =
        get_string_or_default(body, "symbol", "UNKNOWN");

    if (symbol == "UNKNOWN") {
        throw std::runtime_error("symbol is required for position update");
    }

    bool has_position = false;
    if (body.contains("has_position") && !body["has_position"].is_null()) {
        has_position = body["has_position"].get<bool>();
    }

    double average_buy_price =
        get_number_or_default(body, "average_buy_price", 0.0);

    int quantity = 0;
    if (body.contains("quantity") && !body["quantity"].is_null()) {
        quantity = body["quantity"].get<int>();
    }

    risk_manager.updatePosition(
        symbol,
        has_position,
        average_buy_price,
        quantity
    );
}

void StrategyEngine::addTickPrice(SymbolState& state, double price) {
    std::size_t current_size = state.tick_prices.size();

    // ===============================
    // Z-score rolling cache 갱신
    // ===============================
    int z_window = config.zscore_window;

    if (z_window > 0) {
        if (current_size >= static_cast<std::size_t>(z_window)) {
            double old_value =
                state.tick_prices.nthFromBack(static_cast<std::size_t>(z_window - 1));

            state.zscore_sum -= old_value;
            state.zscore_sum_sq -= old_value * old_value;
        }
        else {
            state.zscore_count++;
        }

        state.zscore_sum += price;
        state.zscore_sum_sq += price * price;
    }

    // ===============================
    // Tick MA short rolling cache 갱신
    // ===============================
    int short_window = config.tick_ma_short_window;

    if (short_window > 0) {
        if (current_size >= static_cast<std::size_t>(short_window)) {
            double old_value =
                state.tick_prices.nthFromBack(static_cast<std::size_t>(short_window - 1));

            state.tick_ma_short_sum -= old_value;
        }
        else {
            state.tick_ma_short_count++;
        }

        state.tick_ma_short_sum += price;
    }

    // ===============================
    // Tick MA long rolling cache 갱신
    // ===============================
    int long_window = config.tick_ma_long_window;

    if (long_window > 0) {
        if (current_size >= static_cast<std::size_t>(long_window)) {
            double old_value =
                state.tick_prices.nthFromBack(static_cast<std::size_t>(long_window - 1));

            state.tick_ma_long_sum -= old_value;
        }
        else {
            state.tick_ma_long_count++;
        }

        state.tick_ma_long_sum += price;
    }

    // 실제 가격 저장
    state.tick_prices.push(price);
}

double StrategyEngine::sumVector(const std::vector<double>& values) const {
    return std::accumulate(values.begin(), values.end(), 0.0);
}

std::vector<SignalResult> StrategyEngine::runAvailableStrategies(
    const SymbolState& state
) {
    std::vector<SignalResult> results;

    if (state.tick_prices.size() >= static_cast<std::size_t>(config.zscore_window)) {
        results.push_back(runZScoreStrategy(state, config));
    }

    if (state.tick_prices.size() >= static_cast<std::size_t>(config.tick_ma_long_window)) {
        results.push_back(runTickMAStrategy(state, config));
    }

    if (state.has_execution_data) {
        results.push_back(runExecutionStrengthStrategy(state, config));
    }

    if (state.has_orderbook_data) {
        results.push_back(runOrderbookImbalanceStrategy(state, config));
    }

    return results;
}

SignalResult StrategyEngine::combineResults(
    const std::vector<SignalResult>& results
) {
    if (results.empty()) {
        return {
            "COMBINED",
            "HOLD",
            0.0,
            1.0,
            "Not enough data to run any strategy"
        };
    }

    double weighted_sum = 0.0;
    double total_weight = 0.0;

    std::string reason = "Combined strategies: ";

    for (const auto& result : results) {
        double clipped_score =
            std::clamp(
                result.score,
                config.score_clip_min,
                config.score_clip_max
            );

        weighted_sum += clipped_score * result.weight;
        total_weight += result.weight;

        reason += result.strategy_name;
        reason += "=";
        reason += result.signal;
        reason += "(";
        reason += std::to_string(clipped_score);
        reason += "); ";
    }

    double final_score = 0.0;

    if (total_weight > 0.0) {
        final_score = weighted_sum / total_weight;
    }

    std::string final_signal = "HOLD";

    if (final_score >= config.final_buy_threshold) {
        final_signal = "BUY";
    }
    else if (final_score <= config.final_sell_threshold) {
        final_signal = "SELL";
    }

    return {
        "COMBINED",
        final_signal,
        final_score,
        1.0,
        reason
    };
}