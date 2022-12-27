//
// Created by particleg on 2021/9/24.
//

#pragma once

#include <drogon/HttpController.h>
#include <plugins/PlayerManager.h>
#include <structures/ExceptionHandlers.h>

namespace studio26f::api::v1 {
    class Auth :
            public drogon::HttpController<Auth>,
            public structures::ResponseJsonHandler<Auth> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        Auth();

        METHOD_LIST_BEGIN
            METHOD_ADD(Auth::check, "/check", drogon::Get, "studio26f::filters::CheckAccessToken");
            METHOD_ADD(Auth::refresh, "/refresh", drogon::Get, "studio26f::filters::CheckRefreshToken");
            METHOD_ADD(
                    Auth::verifyEmail,
                    "/verify/email",
                    drogon::Post,
                    "studio26f::filters::AuthVerifyEmail",
                    "studio26f::filters::LimitVerifyEmail",
                    "studio26f::filters::LimitIp"
            );
            METHOD_ADD(Auth::seedEmail, "/seed/email", drogon::Post, "studio26f::filters::AuthSeedEmail");
            METHOD_ADD(
                    Auth::loginEmail,
                    "/login/email",
                    drogon::Post,
                    "studio26f::filters::AuthLoginEmail",
                    "studio26f::filters::LimitLoginEmail",
                    "studio26f::filters::LimitIp"
            );
            METHOD_ADD(
                    Auth::resetEmail,
                    "/reset/email",
                    drogon::Put,
                    "studio26f::filters::AuthResetEmail",
                    "studio26f::filters::LimitLoginEmail",
                    "studio26f::filters::LimitIp"
            );
            METHOD_ADD(
                    Auth::migrateEmail,
                    "/migrate/email",
                    drogon::Put,
                    "studio26f::filters::CheckAccessToken",
                    "studio26f::filters::AuthMigrateEmail");
            METHOD_ADD(
                    Auth::deactivateEmail,
                    "/deactivate/email",
                    drogon::Post,
                    "studio26f::filters::CheckAccessToken",
                    "studio26f::filters::AuthDeactivateEmail");
        METHOD_LIST_END

        void check(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void
        refresh(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void
        verifyEmail(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void
        seedEmail(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void
        loginEmail(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void
        resetEmail(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void migrateEmail(const drogon::HttpRequestPtr &req,
                          std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void deactivateEmail(const drogon::HttpRequestPtr &req,
                             std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    private:
        plugins::PlayerManager *_playerManager;
    };
}