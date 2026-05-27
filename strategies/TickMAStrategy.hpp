#pragma once

#include "SignalResult.hpp"
#include "SymbolState.hpp"

// 틱 이동평균 모멘텀 전략
SignalResult runTickMAStrategy(
    const SymbolState& state,
    int short_window = 5,
    int long_window = 30
);