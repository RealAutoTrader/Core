#pragma once

struct StrategyConfig {
    // ===============================
    // Z-score 평균회귀 전략 파라미터
    // ===============================
    int zscore_window = 20;
    double zscore_entry_threshold = 2.0;
    double zscore_exit_threshold = 0.5;
    double zscore_weight = 0.25;

    // ===============================
    // 체결강도 전략 파라미터
    // ===============================
    double execution_base_strength = 100.0;
    double execution_strength_scale = 20.0;

    double execution_buy_strong = 125.0;
    double execution_sell_strong = 75.0;

    double execution_buy_moderate = 115.0;
    double execution_sell_moderate = 85.0;

    double execution_price_positive_threshold = 0.3;
    double execution_price_negative_threshold = -0.3;

    double execution_weight = 0.30;

    // ===============================
    // 호가 불균형 전략 파라미터
    // ===============================
    double orderbook_strong_threshold = 0.25;
    double orderbook_weak_threshold = 0.18;
    double orderbook_scale = 0.20;

    double orderbook_price_check_threshold = 0.2;

    double orderbook_weight = 0.30;

    // ===============================
    // 틱 이동평균 전략 파라미터
    // ===============================
    int tick_ma_short_window = 5;
    int tick_ma_long_window = 30;
    double tick_ma_gap_scale = 0.003;

    double tick_ma_weight = 0.15;

    // ===============================
    // 최종 통합 신호 기준
    // ===============================
    double final_buy_threshold = 0.5;
    double final_sell_threshold = -0.5;

    // 특정 전략 점수가 너무 커져서 최종 결과를 지배하지 않도록 제한
    double score_clip_min = -2.0;
    double score_clip_max = 2.0;
};