#include "ZScoreStrategy.hpp"

#include <cmath>
#include <deque>
#include <stdexcept>

static double mean_last_n(const std::deque<double>& data, int n) {
    if (n <= 0) {
        throw std::runtime_error("Window must be positive");
    }

    if (data.size() < static_cast<size_t>(n)) {
        throw std::runtime_error("Not enough data for mean");
    }

    double sum = 0.0;
    auto start = data.end() - n;

    for (auto it = start; it != data.end(); ++it) {
        sum += *it;
    }

    return sum / n;
}

static double stddev_last_n(
    const std::deque<double>& data,
    int n,
    double mean
) {
    if (data.size() < static_cast<size_t>(n)) {
        throw std::runtime_error("Not enough data for stddev");
    }

    double variance = 0.0;
    auto start = data.end() - n;

    for (auto it = start; it != data.end(); ++it) {
        double diff = *it - mean;
        variance += diff * diff;
    }

    variance /= n;
    return std::sqrt(variance);
}

SignalResult runZScoreStrategy(
    const SymbolState& state,
    const StrategyConfig& config
) {
    int window = config.zscore_window;

    double mean = mean_last_n(state.tick_prices, window);
    double stddev = stddev_last_n(state.tick_prices, window, mean);

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