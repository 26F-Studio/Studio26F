//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <structures/ExceptionHandlers.h>

/**
 * @brief This filter checks "Auth::loginEmail" request body.
 * @param email: String
 * @param code: String | Optional
 * @param password: String | Optional
 * @return requestJson: in request attributes
 */

namespace studio26f::filters {
    class AuthLoginEmail :
            public drogon::HttpFilter<AuthLoginEmail>,
            public structures::RequestJsonHandler<AuthLoginEmail> {
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