#pragma once

#include "SignalResult.hpp"
#include "SymbolState.hpp"
#include "StrategyConfig.hpp"

// 틱 이동평균 모멘텀 전략
SignalResult runTickMAStrategy(
    const SymbolState& state,
    const StrategyConfig& config
);