#pragma once

#include "json.hpp"

#include <string>
#include <vector>

using json = nlohmann::json;

// JSON에서 숫자 값을 읽는다. 없으면 기본값 반환
double get_number_or_default(
    const json& body,
    const std::string& key,
    double default_value
);

// JSON에서 문자열 값을 읽는다. 없으면 기본값 반환
std::string get_string_or_default(
    const json& body,
    const std::string& key,
    const std::string& default_value
);

// JSON 배열을 vector<double>로 읽는다. 없거나 배열이 아니면 빈 vector 반환
std::vector<double> get_double_vector_or_empty(
    const json& body,
    const std::string& key
);