#include "ExecutionStrengthStrategy.hpp"

#include <string>

SignalResult runExecutionStrengthStrategy(const SymbolState& state) {
    /*
      체결강도는 100을 중립 기준으로 본다.
      100보다 크면 매수 체결 우위, 100보다 작으면 매도 체결 우위로 해석한다.
    */
    const double base_strength = 100.0;

    /*
      체결강도가 기준값에서 20만큼 벗어나면 전략 점수 1로 환산한다.
      이 값은 초기 전략 설계값이며, 백테스트를 통해 조정 가능하다.
    */
    const double strength_scale = 20.0;

    // 실제 통계적 Z-score는 아니고, 체결강도를 정규화한 전략 점수
    double strength_score =
        (state.execution_strength - base_strength) / strength_scale;

    // 가격 등락률과 거래량 증가율을 보조 점수로 변환
    double price_score = state.price_change_rate / 1.0;
    double volume_score = (state.volume_change_rate - 1.0) * 1.5;

    /*
      체결강도를 핵심 지표로 보고 70% 반영한다.
      가격 등락률과 거래량 증가율은 보조 지표로 각각 20%, 10% 반영한다.
    */
    double final_score =
        0.70 * strength_score +
        0.20 * price_score +
        0.10 * volume_score;

    std::string signal = "HOLD";
    std::string reason = "Execution strength is neutral";

    if (state.execution_strength >= 125.0 &&
        state.price_change_rate >= 0.0 &&
        final_score >= 1.0) {
        signal = "BUY";
        reason = "Strong buy execution momentum";
    }
    else if (state.execution_strength <= 75.0 &&
             state.price_change_rate <= 0.0 &&
             final_score <= -1.0) {
        signal = "SELL";
        reason = "Strong sell execution momentum";
    }
    else if (state.execution_strength >= 115.0 &&
             state.price_change_rate > 0.3) {
        signal = "BUY";
        reason = "Moderate buy execution strength with positive price momentum";
    }
    else if (state.execution_strength <= 85.0 &&
             state.price_change_rate < -0.3) {
        signal = "SELL";
        reason = "Moderate sell execution strength with negative price momentum";
    }

    return {
        "EXECUTION_STRENGTH",
        signal,
        final_score,
        0.30,
        reason
    };
}