#include "ZScoreStrategy.hpp"

#include <algorithm>
#include <cmath>
#include <string>

SignalResult runZScoreStrategy(
    const SymbolState& state,
    const StrategyConfig& config
) {
    int window = config.zscore_window;

    if (window <= 0 ||
        state.zscore_count < static_cast<std::size_t>(window)) {
        return {
            "Z_SCORE",
            "HOLD",
            0.0,
            config.zscore_weight,
            "Not enough data for Z-score"
        };
    }

    double mean = state.zscore_sum / window;

    double variance =
        (state.zscore_sum_sq / window) - (mean * mean);

    // 부동소수점 오차로 인해 아주 작은 음수가 나올 수 있으므로 보정
    variance = std::max(variance, 0.0);

    double stddev = std::sqrt(variance);

    if (stddev == 0.0) {
        return {
            "Z_SCORE",
            "HOLD",
            0.0,
            config.zscore_weight,
            "Z-score stddev is zero"
        };
    }

    double current_price = state.tick_prices.back();
    double z_score = (current_price - mean) / stddev;

    std::string signal = "HOLD";
    std::string reason = "Z-score is within normal range";

    if (z_score < -config.zscore_entry_threshold) {
        signal = "BUY";
        reason = "Z-score below lower threshold";
    }
    else if (z_score > config.zscore_entry_threshold) {
        signal = "SELL";
        reason = "Z-score above upper threshold";
    }
    else if (std::abs(z_score) < config.zscore_exit_threshold) {
        signal = "HOLD";
        reason = "Z-score reverted near mean";
    }

    // 평균회귀 전략은 z_score가 낮을수록 BUY 방향
    double score = -z_score;

    return {
        "Z_SCORE",
        signal,
        score,
        config.zscore_weight,
        reason
    };
}