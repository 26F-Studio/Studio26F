//
// Created by particleg on 2021/9/29.
//

#pragma once

#include <string>
#include <json/value.h>

namespace studio26f::structures {
    class RedisToken {
    public:
        RedisToken(
                std::string refresh,
                std::string access
        );

        RedisToken(RedisToken &&redisToken) noexcept;

        [[nodiscard]] std::string &access();

        [[nodiscard]] Json::Value parse() const;

    private:
        std::string _refreshToken, _accessToken;
    };
}
