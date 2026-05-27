#include "ExecutionStrengthStrategy.hpp"

#include <string>

SignalResult runExecutionStrengthStrategy(
    const SymbolState& state,
    const StrategyConfig& config
) {
    double strength_score =
        (state.execution_strength - config.execution_base_strength)
        / config.execution_strength_scale;

    double price_score = state.price_change_rate / 1.0;
    double volume_score = (state.volume_change_rate - 1.0) * 1.5;

    double final_score =
        0.70 * strength_score +
        0.20 * price_score +
        0.10 * volume_score;

    std::string signal = "HOLD";
    std::string reason = "Execution strength is neutral";

    if (state.execution_strength >= config.execution_buy_strong &&
        state.price_change_rate >= 0.0 &&
        final_score >= 1.0) {
        signal = "BUY";
        reason = "Strong buy execution momentum";
    }
    else if (state.execution_strength <= config.execution_sell_strong &&
             state.price_change_rate <= 0.0 &&
             final_score <= -1.0) {
        signal = "SELL";
        reason = "Strong sell execution momentum";
    }
    else if (state.execution_strength >= config.execution_buy_moderate &&
             state.price_change_rate > config.execution_price_positive_threshold) {
        signal = "BUY";
        reason = "Moderate buy execution strength with positive price momentum";
    }
    else if (state.execution_strength <= config.execution_sell_moderate &&
             state.price_change_rate < config.execution_price_negative_threshold) {
        signal = "SELL";
        reason = "Moderate sell execution strength with negative price momentum";
    }

    return {
        "EXECUTION_STRENGTH",
        signal,
        final_score,
        config.execution_weight,
        reason
    };
}