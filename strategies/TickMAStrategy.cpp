#include "TickMAStrategy.hpp"

#include <stdexcept>
#include <string>

SignalResult runTickMAStrategy(
    const SymbolState& state,
    const StrategyConfig& config
) {
    int short_window = config.tick_ma_short_window;
    int long_window = config.tick_ma_long_window;

    if (short_window <= 0 || long_window <= 0) {
        throw std::runtime_error("Moving average windows must be positive");
    }

    if (short_window >= long_window) {
        throw std::runtime_error("short_window must be smaller than long_window");
    }

    if (state.tick_ma_short_count < static_cast<std::size_t>(short_window) ||
        state.tick_ma_long_count < static_cast<std::size_t>(long_window)) {
        return {
            "TICK_MA",
            "HOLD",
            0.0,
            config.tick_ma_weight,
            "Not enough data for tick moving average"
        };
    }

    double short_ma = state.tick_ma_short_sum / short_window;
    double long_ma = state.tick_ma_long_sum / long_window;

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