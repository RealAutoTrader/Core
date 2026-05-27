#pragma once

#include <chrono>
#include <deque>
#include <string>
#include <vector>

// 종목별로 C++ 서버가 유지할 상태 정보
struct SymbolState {
    std::string symbol;

    // 최근 체결 가격 저장
    // Z-score 전략과 틱 이동평균 전략에서 사용
    std::deque<double> tick_prices;

    // 체결강도 전략에서 사용하는 최신 데이터
    bool has_execution_data = false;
    double execution_strength = 100.0;
    double price_change_rate = 0.0;
    double volume_change_rate = 1.0;

    // 호가 불균형 전략에서 사용하는 최신 호가 데이터
    bool has_orderbook_data = false;
    std::vector<double> bid_prices;
    std::vector<double> bid_volumes;
    std::vector<double> ask_prices;
    std::vector<double> ask_volumes;

    // 마지막 이벤트 수신 시간
    // 10분 동안 이벤트가 없으면 상태 삭제에 사용
    std::chrono::steady_clock::time_point last_update;

    // 최근 가격은 최대 60개까지만 저장
    static constexpr size_t MAX_TICK_SIZE = 60;
};