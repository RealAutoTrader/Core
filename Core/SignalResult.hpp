#pragma once

#include <string>

// 각 전략의 계산 결과를 공통 형식으로 표현하는 구조체
struct SignalResult {
    std::string strategy_name; // 전략 이름: Z_SCORE, EXECUTION_STRENGTH, ORDERBOOK_IMBALANCE, TICK_MA, COMBINED
    std::string signal;        // 최종 신호: BUY, SELL, HOLD
    double score;              // 전략 점수. BUY 방향은 양수, SELL 방향은 음수
    double weight;             // 통합 점수 계산 시 사용할 전략 가중치
    std::string reason;        // 신호가 발생한 이유
};