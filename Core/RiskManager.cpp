#include "RiskManager.hpp"

SignalResult RiskManager::applyRiskRules(
    const std::string& symbol,
    const SignalResult& signal,
    const SymbolState& state
) {
    RiskState& risk_state = risk_states[symbol];

    // HOLD는 주문 신호가 아니므로 그대로 통과
    if (signal.signal == "HOLD") {
        return signal;
    }

    // 1. 하루 주문 횟수 제한
    if (risk_state.daily_order_count >= config.max_daily_orders_per_symbol) {
        SignalResult blocked = signal;
        blocked.signal = "HOLD";
        blocked.score = 0.0;
        blocked.reason += " Risk blocked: daily order limit reached.";
        return blocked;
    }

    // 2. 연속 손실 제한
    if (risk_state.consecutive_losses >= config.max_consecutive_losses) {
        SignalResult blocked = signal;
        blocked.signal = "HOLD";
        blocked.score = 0.0;
        blocked.reason += " Risk blocked: too many consecutive losses.";
        return blocked;
    }

    // 3. 중복 매수 방지
    if (config.block_duplicate_buy &&
        signal.signal == "BUY" &&
        risk_state.has_position) {
        SignalResult blocked = signal;
        blocked.signal = "HOLD";
        blocked.score = 0.0;
        blocked.reason += " Risk blocked: already holding position.";
        return blocked;
    }

    // 4. 손실 제한
    // 보유 중이고 손실률이 기준 이하이면 SELL 우선
    if (risk_state.has_position) {
        double loss_rate = calculateLossRate(risk_state, state);

        if (loss_rate <= config.max_loss_rate && signal.signal != "SELL") {
            SignalResult forced_sell = signal;
            forced_sell.signal = "SELL";
            forced_sell.score = -1.0;
            forced_sell.reason += " Risk override: max loss limit reached, force SELL.";
            return forced_sell;
        }
    }

    return signal;
}

void RiskManager::updatePosition(
    const std::string& symbol,
    bool has_position,
    double average_buy_price,
    int quantity
) {
    RiskState& risk_state = risk_states[symbol];

    risk_state.has_position = has_position;
    risk_state.average_buy_price = average_buy_price;
    risk_state.position_quantity = quantity;
}

void RiskManager::recordOrder(const std::string& symbol) {
    RiskState& risk_state = risk_states[symbol];

    risk_state.daily_order_count++;
    risk_state.last_order_time = std::chrono::system_clock::now();
}

double RiskManager::calculateLossRate(
    const RiskState& risk_state,
    const SymbolState& state
) const {
    if (!risk_state.has_position ||
        risk_state.average_buy_price <= 0.0 ||
        state.tick_prices.empty()) {
        return 0.0;
    }

    double current_price = state.tick_prices.back();

    return (current_price - risk_state.average_buy_price)
        / risk_state.average_buy_price;
}