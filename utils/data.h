//
// Created by ParticleG on 2022/2/3.
//

#pragma once

#include <drogon/drogon.h>

namespace studio26f::internal {
    template<typename T>
    concept JsonTypes =
    std::convertible_to<T, bool> ||
    std::convertible_to<T, Json::Int> ||
    std::convertible_to<T, Json::Int64> ||
    std::convertible_to<T, Json::UInt> ||
    std::convertible_to<T, Json::UInt64> ||
    std::convertible_to<T, double> ||
    std::convertible_to<T, Json::String>;
};

namespace studio26f::utils::data {
    std::string randomString(uint64_t length);

    uint64_t randomUniform(uint64_t start = 0, uint64_t size = 99999999);
}



