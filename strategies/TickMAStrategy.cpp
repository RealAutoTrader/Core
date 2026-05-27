#include "TickMAStrategy.hpp"

#include <cmath>
#include <deque>
#include <stdexcept>
#include <string>

static double mean_last_n(const std::deque<double>& data, int n) {
    if (n <= 0) {
        throw std::runtime_error("Window must be positive");
    }

    if (data.size() < static_cast<size_t>(n)) {
        throw std::runtime_error("Not enough data for moving average");
    }

    double sum = 0.0;
    auto start = data.end() - n;

    for (auto it = start; it != data.end(); ++it) {
        sum += *it;
    }

    return sum / n;
}

SignalResult runTickMAStrategy(
    const SymbolState& state,
    const StrategyConfig& config
) {
    int short_window = config.tick_ma_short_window;
    int long_window = config.tick_ma_long_window;

    if (short_window >= long_window) {
        throw std::runtime_error("short_window must be smaller than long_window");
    }

    double short_ma = mean_last_n(state.tick_prices, short_window);
    double long_ma = mean_last_n(state.tick_prices, long_window);

    if (long_ma == 0.0) {
        return {
            "TICK_MA",
            "HOLD",
            0.0,
            config.tick_ma_weight,
            "Long moving average is zero"
        };
    }

    double ma_gap_rate = (short_ma - long_ma) / long_ma;
    double final_score = ma_gap_rate / config.tick_ma_gap_scale;

    std::string signal = "HOLD";
    std::string reason = "Tick moving averages are neutral";

    if (final_score >= 1.0) {
        signal = "BUY";
        reason = "Short tick MA is above long tick MA";
    }
    else if (final_score <= -1.0) {
        signal = "SELL";
        reason = "Short tick MA is below long tick MA";
    }

    return {
        "TICK_MA",
        signal,
        final_score,
        config.tick_ma_weight,
        reason
    };
}