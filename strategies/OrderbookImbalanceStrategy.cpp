#include "OrderbookImbalanceStrategy.hpp"

#include <stdexcept>
#include <string>

SignalResult runOrderbookImbalanceStrategy(
    const SymbolState& state,
    const StrategyConfig& config
) {
    if (!state.has_orderbook_data) {
        return {
            "ORDERBOOK_IMBALANCE",
            "HOLD",
            0.0,
            config.orderbook_weight,
            "Orderbook data is empty"
        };
    }

    double total_bid_volume = state.total_bid_volume;
    double total_ask_volume = state.total_ask_volume;

    if (total_bid_volume < 0.0 || total_ask_volume < 0.0) {
        throw std::runtime_error("Orderbook volumes must be non-negative");
    }

    double total = total_bid_volume + total_ask_volume;

    if (total <= 0.0) {
        throw std::runtime_error("Total orderbook volume is zero");
    }

    double imbalance =
        (total_bid_volume - total_ask_volume) / total;

    double imbalance_score = imbalance / config.orderbook_scale;
    double price_score = state.price_change_rate / 1.0;

    double final_score =
        0.85 * imbalance_score +
        0.15 * price_score;

    std::string signal = "HOLD";
    std::string reason = "Orderbook imbalance is neutral";

    if (imbalance >= config.orderbook_strong_threshold &&
        state.price_change_rate >= -config.orderbook_price_check_threshold) {
        signal = "BUY";
        reason = "Bid-side orderbook volume is dominant";
    }
    else if (imbalance <= -config.orderbook_strong_threshold &&
             state.price_change_rate <= config.orderbook_price_check_threshold) {
        signal = "SELL";
        reason = "Ask-side orderbook volume is dominant";
    }
    else if (imbalance >= config.orderbook_weak_threshold &&
             state.price_change_rate > config.orderbook_price_check_threshold) {
        signal = "BUY";
        reason = "Positive orderbook imbalance with price confirmation";
    }
    else if (imbalance <= -config.orderbook_weak_threshold &&
             state.price_change_rate < -config.orderbook_price_check_threshold) {
        signal = "SELL";
        reason = "Negative orderbook imbalance with price confirmation";
    }

    return {
        "ORDERBOOK_IMBALANCE",
        signal,
        final_score,
        config.orderbook_weight,
        reason
    };
}