#pragma once

#include "SignalResult.hpp"
#include "SymbolState.hpp"

// 체결강도 기반 모멘텀 전략
SignalResult runExecutionStrengthStrategy(const SymbolState& state);