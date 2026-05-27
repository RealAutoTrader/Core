#include "StrategyEngine.hpp"

#include "JsonUtils.hpp"

#include "ZScoreStrategy.hpp"
#include "ExecutionStrengthStrategy.hpp"
#include "OrderbookImbalanceStrategy.hpp"
#include "TickMAStrategy.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

SignalResult StrategyEngine::handleEvent(const json& body) {
    // 요청이 들어올 때마다 10분 이상 이벤트가 없는 종목 삭제
    cleanupExpiredStates();

    std::string symbol =
        get_string_or_default(body, "symbol", "UNKNOWN");

    if (symbol == "UNKNOWN") {
        throw std::runtime_error("symbol is required");
    }

    // 해당 종목의 상태를 가져오거나 새로 생성
    SymbolState& state = states[symbol];
    state.symbol = symbol;
    state.last_update = std::chrono::steady_clock::now();

    // 이벤트 데이터 반영
    updateState(state, body);

    // 현재 가능한 전략 실행
    std::vector<SignalResult> results =
        runAvailableStrategies(state);

    // 실행된 전략 결과 통합
    return combineResults(results);
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
        /*
          event_type이 없더라도 들어온 필드를 기준으로 최대한 갱신한다.
          테스트 편의성을 위한 처리이다.
        */
        updateFlexibleState(state, body);
    }
}

void StrategyEngine::updateTradeState(SymbolState& state, const json& body) {
    if (body.contains("price") && !body["price"].is_null()) {
        double price = body["price"].get<double>();

        if (price > 0.0) {
            state.tick_prices.push_back(price);

            if (state.tick_prices.size() > SymbolState::MAX_TICK_SIZE) {
                state.tick_prices.pop_front();
            }
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
        state.has_orderbook_data = true;
    }

    if (body.contains("price_change_rate") &&
        !body["price_change_rate"].is_null()) {
        state.price_change_rate =
            body["price_change_rate"].get<double>();
    }
}

void StrategyEngine::updateFlexibleState(SymbolState& state, const json& body) {
    // price가 있으면 체결 이벤트처럼 처리
    if (body.contains("price")) {
        updateTradeState(state, body);
    }

    // 호가 관련 필드가 있으면 호가 이벤트처럼 처리
    if (body.contains("bid_volumes") || body.contains("ask_volumes") ||
        body.contains("bid_prices") || body.contains("ask_prices")) {
        updateOrderbookState(state, body);
    }

    /*
      기존 테스트 호환:
      tick_prices 배열이 통째로 들어오면 해당 가격들을 모두 상태에 추가한다.
    */
    if (body.contains("tick_prices") && body["tick_prices"].is_array()) {
        std::vector<double> prices =
            body["tick_prices"].get<std::vector<double>>();

        for (double price : prices) {
            if (price > 0.0) {
                state.tick_prices.push_back(price);

                if (state.tick_prices.size() > SymbolState::MAX_TICK_SIZE) {
                    state.tick_prices.pop_front();
                }
            }
        }
    }
}

std::vector<SignalResult> StrategyEngine::runAvailableStrategies(
    const SymbolState& state
) {
    std::vector<SignalResult> results;

    // Z-score는 최근 가격 20개 이상 필요
    if (state.tick_prices.size() >= 20) {
        results.push_back(runZScoreStrategy(state, 20));
    }

    // 이동평균은 최근 가격 30개 이상 필요
    if (state.tick_prices.size() >= 30) {
        results.push_back(runTickMAStrategy(state, 5, 30));
    }

    // 체결강도 데이터가 들어온 적 있으면 실행
    if (state.has_execution_data) {
        results.push_back(runExecutionStrengthStrategy(state));
    }

    // 호가 데이터가 들어온 적 있으면 실행
    if (state.has_orderbook_data) {
        results.push_back(runOrderbookImbalanceStrategy(state));
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
        /*
          특정 전략 점수가 너무 커져 최종 점수를 지배하지 않도록 제한한다.
          예: score가 5.0이어도 2.0까지만 반영.
        */
        double clipped_score =
            std::clamp(result.score, -2.0, 2.0);

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

    if (final_score >= 0.5) {
        final_signal = "BUY";
    }
    else if (final_score <= -0.5) {
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