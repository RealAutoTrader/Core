#define _WIN32_WINNT 0x0A00
#define _CRT_SECURE_NO_WARNINGS

#include "httplib.h"
#include "json.hpp"

#include "StrategyEngine.hpp"

#include <chrono>
#include <iostream>

using json = nlohmann::json;

int main() {
    httplib::Server svr;
    StrategyEngine engine;

    svr.Post("/signal", [&](const httplib::Request& req, httplib::Response& res) {
        auto start =
            std::chrono::high_resolution_clock::now();

        try {
            json body = json::parse(req.body);

            SignalResult result =
                engine.handleEvent(body);

            auto end =
                std::chrono::high_resolution_clock::now();

            auto latency_us =
                std::chrono::duration_cast<std::chrono::microseconds>(
                    end - start
                ).count();

            json response = {
                {"signal", result.signal},

                /*
                  기존 Spring DTO 호환을 위해 z_score 필드를 사용한다.
                  여기서는 실제 통계적 Z-score가 아니라 통합 전략 점수이다.
                */
                {"z_score", result.score},

                /*
                  통합 전략에서는 특정 하나의 rolling_mean/stddev를 대표하기 어렵다.
                  기존 DTO 호환을 위해 0.0으로 둔다.
                */
                {"rolling_mean", 0.0},
                {"rolling_stddev", 0.0},

                {"reason", result.reason},
                {"strategy", result.strategy_name},
                {"active_symbols", engine.getStateCount()},
                {"latency_us", latency_us}
            };

            res.set_content(response.dump(2), "application/json");
        }
        catch (const std::exception& e) {
            json error = {
                {"error", e.what()}
            };

            res.status = 400;
            res.set_content(error.dump(2), "application/json");
        }
    });

    std::cout << "Integrated C++ Strategy Server running on port 8081"
              << std::endl;

    svr.listen("0.0.0.0", 8081);

    return 0;
}