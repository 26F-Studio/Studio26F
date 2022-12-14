//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <helpers/I18nHelper.h>

/**
 * @brief This filter checks header "x-access-token" and set attribute "playerId"
 * @param x-access-token: in header
 * @return playerId: int64_t
 */

namespace studio26f::filters {
    class CheckAccessToken :
            public drogon::HttpFilter<CheckAccessToken>,
            public helpers::I18nHelper<CheckAccessToken> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        void doFilter(
                const drogon::HttpRequestPtr &req,
                drogon::FilterCallback &&failedCb,
                drogon::FilterChainCallback &&nextCb
        ) override;
    };
}