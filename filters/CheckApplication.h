//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <helpers/I18nHelper.h>

/**
 * @brief This filter checks header "x-requested-with" and set attribute "app".
 * @param x-access-token: in header
 * @return app: enum class
 */

namespace studio26f::filters {
    class CheckApplication :
            public drogon::HttpFilter<CheckApplication>,
            public helpers::I18nHelper<CheckApplication> {
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