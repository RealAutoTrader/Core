#pragma once

#include "PriceRingBuffer.hpp"

#include <chrono>
#include <string>
#include <vector>

// 종목별로 C++ 서버가 유지할 상태 정보
struct SymbolState {
    std::string symbol;

    // 최근 체결 가격 저장
    // 기존 std::deque 대신 고정 크기 Ring Buffer 사용
    PriceRingBuffer tick_prices;

    // ===============================
    // Rolling 계산 캐시
    // ===============================

    // Z-score window용 rolling sum / squared sum
    double zscore_sum = 0.0;
    double zscore_sum_sq = 0.0;
    std::size_t zscore_count = 0;

    // Tick MA short window용 rolling sum
    double tick_ma_short_sum = 0.0;
    std::size_t tick_ma_short_count = 0;

    // Tick MA long window용 rolling sum
    double tick_ma_long_sum = 0.0;
    std::size_t tick_ma_long_count = 0;

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

    // 호가 잔량 합계 캐싱
    double total_bid_volume = 0.0;
    double total_ask_volume = 0.0;

    // 마지막 이벤트 수신 시간
    // 10분 동안 이벤트가 없으면 상태 삭제에 사용
    std::chrono::steady_clock::time_point last_update;
};