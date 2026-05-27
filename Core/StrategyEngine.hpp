#pragma once

#include "SignalResult.hpp"
#include "SymbolState.hpp"
#include "json.hpp"

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

class StrategyEngine {
private:
    // symbol별 상태 저장
    std::unordered_map<std::string, SymbolState> states;

    // 10분 동안 이벤트가 없으면 상태 삭제
    const std::chrono::minutes STATE_TIMEOUT = std::chrono::minutes(10);

public:
    // Spring에서 이벤트가 들어오면 호출되는 메인 함수
    SignalResult handleEvent(const json& body);

    // 현재 상태를 유지 중인 종목 수
    size_t getStateCount() const;

private:
    // 오래된 종목 상태 삭제
    void cleanupExpiredStates();

    // 이벤트 타입에 따라 상태 갱신
    void updateState(SymbolState& state, const json& body);
    void updateTradeState(SymbolState& state, const json& body);
    void updateOrderbookState(SymbolState& state, const json& body);
    void updateFlexibleState(SymbolState& state, const json& body);

    // 현재 상태에서 실행 가능한 전략만 실행
    std::vector<SignalResult> runAvailableStrategies(const SymbolState& state);

    // 전략 결과 통합
    SignalResult combineResults(const std::vector<SignalResult>& results);
};