#include "OrderbookImbalanceStrategy.hpp"

#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

static double sum_vector(const std::vector<double>& data) {
    return std::accumulate(data.begin(), data.end(), 0.0);
}

SignalResult runOrderbookImbalanceStrategy(const SymbolState& state) {
    if (state.bid_volumes.empty() || state.ask_volumes.empty()) {
        return {
            "ORDERBOOK_IMBALANCE",
            "HOLD",
            0.0,
            0.30,
            "Orderbook data is empty"
        };
    }

    double total_bid_volume = sum_vector(state.bid_volumes);
    double total_ask_volume = sum_vector(state.ask_volumes);

    if (total_bid_volume < 0.0 || total_ask_volume < 0.0) {
        throw std::runtime_error("Orderbook volumes must be non-negative");
    }

    double total = total_bid_volume + total_ask_volume;

    /*
      호가 잔량 총합이 0이면 imbalance 계산식의 분모가 0이 된다.
      즉, 호가 데이터가 비어 있거나 유효하지 않은 입력으로 판단한다.
    */
    if (total <= 0.0) {
        throw std::runtime_error("Total orderbook volume is zero");
    }

    /*
      호가 불균형 계산식
      양수면 매수 잔량 우위, 음수면 매도 잔량 우위이다.
    */
    double imbalance =
        (total_bid_volume - total_ask_volume) / total;

    /*
      imbalance 0.20을 전략 점수 1로 환산한다.
      예: imbalance = 0.20이면 imbalance_score = 1.0
    */
    double imbalance_score = imbalance / 0.20;
    double price_score = state.price_change_rate / 1.0;

    double final_score =
        0.85 * imbalance_score +
        0.15 * price_score;

    std::string signal = "HOLD";
    std::string reason = "Orderbook imbalance is neutral";

    /*
      강한 호가 불균형이 발생한 경우 신호 생성.
      price_change_rate는 실제 가격 흐름이 호가 방향과 크게 충돌하지 않는지 확인하는 보조 조건이다.
    */
    if (imbalance >= 0.25 && state.price_change_rate >= -0.2) {
        signal = "BUY";
        reason = "Bid-side orderbook volume is dominant";
    }
    else if (imbalance <= -0.25 && state.price_change_rate <= 0.2) {
        signal = "SELL";
        reason = "Ask-side orderbook volume is dominant";
    }
    /*
      호가 불균형은 비교적 약하지만 가격 흐름이 같은 방향으로 확인되는 경우 신호 생성.
    */
    else if (imbalance >= 0.18 && state.price_change_rate > 0.2) {
        signal = "BUY";
        reason = "Positive orderbook imbalance with price confirmation";
    }
    else if (imbalance <= -0.18 && state.price_change_rate < -0.2) {
        signal = "SELL";
        reason = "Negative orderbook imbalance with price confirmation";
    }

    return {
        "ORDERBOOK_IMBALANCE",
        signal,
        final_score,
        0.30,
        reason
    };
}