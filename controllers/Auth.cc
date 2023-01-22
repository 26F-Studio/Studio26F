//
// Created by particleg on 2021/9/24.
//

#include <controllers/Auth.h>
#include <magic_enum.hpp>
#include <types/Products.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace studio26f::api::v1;
using namespace studio26f::helpers;
using namespace studio26f::plugins;
using namespace studio26f::structures;
using namespace studio26f::types;

Auth::Auth() : _playerManager(app().getPlugin<PlayerManager>()) {}

void Auth::oauth(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    JsonHelper response(k200OK, ResultCode::Completed);
    handleExceptions([&]() {
        auto request = req->attributes()->get<JsonHelper>("requestJson");
        Json::Value data;
        data["oauthToken"] = _playerManager->oauth(
                request["recaptcha"].asString(),
                req->getPeerAddr(),
                request["product"].asString(),
                req->attributes()->get<int64_t>("playerId")
        );
        const auto accessToken = req->attributes()->get<string>("accessToken");
        if (!accessToken.empty()) {
            data["accessToken"] = accessToken;
            response.setResultCode(ResultCode::Continued);
        }
        response.setData(data);
    }, response);
    response.to(callback);
}

void Auth::check(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    JsonHelper response(k200OK, ResultCode::Completed);
    handleExceptions([&]() {
        Json::Value data;
        data["playerId"] = req->attributes()->get<int64_t>("playerId");
        const auto accessToken = req->attributes()->get<string>("accessToken");
        if (!accessToken.empty()) {
            data["accessToken"] = accessToken;
            response.setResultCode(ResultCode::Continued);
        }
        response.setData(data);
    }, response);
    response.to(callback);
}

void Auth::verifyEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    JsonHelper response(k200OK, ResultCode::Completed);
    handleExceptions([&]() {
        _playerManager->verifyEmail(
                req->attributes()->get<JsonHelper>("requestJson")["email"].asString()
        );
    }, response);
    response.to(callback);
}

void Auth::seedEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    JsonHelper response(k200OK, ResultCode::Completed);
    handleExceptions([&]() {
        response.setData(_playerManager->seedEmail(
                req->attributes()->get<JsonHelper>("requestJson")["email"].asString()
        ));
    }, response);
    response.to(callback);
}

void Auth::loginEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    JsonHelper response(k200OK, ResultCode::Completed);
    handleExceptions([&]() {
        auto request = req->attributes()->get<JsonHelper>("requestJson");
        if (request.check("code", JsonValue::String)) {
            const auto &[accessToken, isNew] = _playerManager->loginEmailCode(
                    request["email"].asString(),
                    request["code"].asString()
            );
            if (isNew) {
                response.setResultCode(ResultCode::Continued);
            }
            Json::Value data;
            data["accessToken"] = accessToken;
            response.setData(data);
        } else {
            Json::Value data;
            data["accessToken"] = _playerManager->loginEmailPassword(
                    request["email"].asString(),
                    request["password"].asString()
            );
            response.setData(data);
        }
    }, response);
    response.to(callback);
}

void Auth::resetEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    JsonHelper response(k200OK, ResultCode::Completed);
    handleExceptions([&]() {
        auto request = req->attributes()->get<JsonHelper>("requestJson");
        _playerManager->resetEmail(
                request["email"].asString(),
                request["code"].asString(),
                request["newPassword"].asString()
        );
    }, response);
    response.to(callback);
}

void Auth::migrateEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    JsonHelper response(k200OK, ResultCode::Completed);
    handleExceptions([&]() {
        auto request = req->attributes()->get<JsonHelper>("requestJson");
        _playerManager->migrateEmail(
                req->attributes()->get<int64_t>("playerId"),
                request["newEmail"].asString(),
                request["code"].asString()
        );
        const auto accessToken = req->attributes()->get<string>("accessToken");
        if (!accessToken.empty()) {
            Json::Value data;
            data["accessToken"] = accessToken;
            response.setResultCode(ResultCode::Continued);
            response.setData(data);
        }
    }, response);
    response.to(callback);
}

void Auth::deactivateEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    JsonHelper response(k200OK, ResultCode::Completed);
    handleExceptions([&]() {
        auto request = req->attributes()->get<JsonHelper>("requestJson");
        _playerManager->deactivateEmail(
                req->attributes()->get<int64_t>("playerId"),
                request["code"].asString()
        );
    }, response);
    response.to(callback);
}