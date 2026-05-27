#pragma once

#include "SignalResult.hpp"
#include "SymbolState.hpp"

// Z-score 평균회귀 전략
SignalResult runZScoreStrategy(
    const SymbolState& state,
    int window = 20
);