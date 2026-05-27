#pragma once

#include "SignalResult.hpp"
#include "SymbolState.hpp"

// 호가 잔량 불균형 전략
SignalResult runOrderbookImbalanceStrategy(const SymbolState& state);