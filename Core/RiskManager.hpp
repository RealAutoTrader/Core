#pragma once

#include "SignalResult.hpp"
#include "SymbolState.hpp"

#include <chrono>
#include <string>
#include <unordered_map>

// 리스크 관리 설정값
struct RiskConfig {
    // 종목별 하루 최대 주문 횟수
    int max_daily_orders_per_symbol = 5;

    // 연속 손실 제한
    int max_consecutive_losses = 3;

    // 손실 제한 기준
    // 예: -0.03 = -3%
    double max_loss_rate = -0.03;

    // 이미 보유 중인 종목에 대해 추가 BUY 차단
    bool block_duplicate_buy = true;
};

// 종목별 리스크 상태
struct RiskState {
    int daily_order_count = 0;
    int consecutive_losses = 0;

    bool has_position = false;
    double average_buy_price = 0.0;
    int position_quantity = 0;

    std::chrono::system_clock::time_point last_order_time;
};

class RiskManager {
private:
    RiskConfig config;
    std::unordered_map<std::string, RiskState> risk_states;

public:
    // 전략 결과에 리스크 관리 규칙을 적용한다.
    SignalResult applyRiskRules(
        const std::string& symbol,
        const SignalResult& signal,
        const SymbolState& state
    );

    // Spring에서 보유 상태를 알려줄 때 사용
    void updatePosition(
        const std::string& symbol,
        bool has_position,
        double average_buy_price,
        int quantity
    );

    // 주문이 실제로 발생했다고 가정할 때 주문 횟수 증가
    void recordOrder(const std::string& symbol);

private:
    double calculateLossRate(
        const RiskState& risk_state,
        const SymbolState& state
    ) const;
};