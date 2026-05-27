#pragma once

#include "SignalResult.hpp"
#include "SymbolState.hpp"
#include "StrategyConfig.hpp"

// Z-score 평균회귀 전략
SignalResult runZScoreStrategy(
    const SymbolState& state,
    const StrategyConfig& config
);