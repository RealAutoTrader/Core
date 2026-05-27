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
    int short_window,
    int long_window
) {
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
            0.15,
            "Long moving average is zero"
        };
    }

    /*
      이동평균 괴리율
      양수면 단기 이동평균이 장기 이동평균보다 높음.
      즉, 최근 흐름이 상승 방향이라는 의미이다.
    */
    double ma_gap_rate = (short_ma - long_ma) / long_ma;

    /*
      괴리율 0.3%를 전략 점수 1로 환산한다.
      ma_gap_rate = 0.003이면 final_score = 1.0
    */
    double final_score = ma_gap_rate / 0.003;

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
        0.15,
        reason
    };
}