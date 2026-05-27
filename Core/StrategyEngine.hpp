#pragma once

#include "SignalResult.hpp"
#include "SymbolState.hpp"
#include "StrategyConfig.hpp"
#include "RiskManager.hpp"
#include "json.hpp"

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

class StrategyEngine {
private:
    std::unordered_map<std::string, SymbolState> states;

    StrategyConfig config;
    RiskManager risk_manager;

    const std::chrono::minutes STATE_TIMEOUT = std::chrono::minutes(10);

public:
    SignalResult handleEvent(const json& body);

    size_t getStateCount() const;

private:
    void cleanupExpiredStates();

    void updateState(SymbolState& state, const json& body);
    void updateTradeState(SymbolState& state, const json& body);
    void updateOrderbookState(SymbolState& state, const json& body);
    void updateFlexibleState(SymbolState& state, const json& body);
    void updatePositionState(const json& body);

    std::vector<SignalResult> runAvailableStrategies(const SymbolState& state);

    SignalResult combineResults(const std::vector<SignalResult>& results);
};