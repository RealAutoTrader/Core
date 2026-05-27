#include "JsonUtils.hpp"

double get_number_or_default(
    const json& body,
    const std::string& key,
    double default_value
) {
    if (!body.contains(key) || body[key].is_null()) {
        return default_value;
    }

    return body[key].get<double>();
}

std::string get_string_or_default(
    const json& body,
    const std::string& key,
    const std::string& default_value
) {
    if (!body.contains(key) || body[key].is_null()) {
        return default_value;
    }

    return body[key].get<std::string>();
}

std::vector<double> get_double_vector_or_empty(
    const json& body,
    const std::string& key
) {
    if (!body.contains(key) || !body[key].is_array()) {
        return {};
    }

    return body[key].get<std::vector<double>>();
}