//
// Created by particleg on 2021/9/24.
//

#include <controllers/Auth.h>
#include <helpers/RequestJson.h>
#include <helpers/ResponseJson.h>
#include <magic_enum.hpp>
#include <types/Applications.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace studio26f::api::v1;
using namespace studio26f::helpers;
using namespace studio26f::plugins;
using namespace studio26f::structures;
using namespace studio26f::types;

Auth::Auth() : _playerManager(app().getPlugin<PlayerManager>()) {}

void Auth::check(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        response.setData(req->attributes()->get<int64_t>("playerId"));
    }, response);
    response.to(callback);
}

void Auth::refresh(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        response.setData(_playerManager->refresh(
                req->attributes()->get<string>("refreshToken")
        ).parse());
    }, response);
    response.to(callback);
}

void Auth::verifyEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        _playerManager->verifyEmail(
                req->attributes()->get<RequestJson>("requestJson")["email"].asString()
        );
    }, response);
    response.to(callback);
}

void Auth::seedEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        response.setData(_playerManager->seedEmail(
                req->attributes()->get<RequestJson>("requestJson")["email"].asString()
        ));
    }, response);
    response.to(callback);
}

void Auth::loginEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        auto request = req->attributes()->get<RequestJson>("requestJson");
        if (request.check("code", JsonValue::String)) {
            const auto &[tokens, isNew] = _playerManager->loginEmailCode(
                    request["email"].asString(),
                    request["code"].asString()
            );
            if (isNew) {
                response.setResultCode(ResultCode::Continued);
            }
            response.setData(tokens.parse());
        } else {
            const auto &tokens = _playerManager->loginEmailPassword(
                    request["email"].asString(),
                    request["password"].asString()
            );
            response.setData(tokens.parse());
        }
    }, response);
    response.to(callback);
}

void Auth::resetEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        auto request = req->attributes()->get<RequestJson>("requestJson");
        _playerManager->resetEmail(
                request["email"].asString(),
                request["code"].asString(),
                request["newPassword"].asString()
        );
    }, response);
    response.to(callback);
}

void Auth::migrateEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        auto request = req->attributes()->get<RequestJson>("requestJson");
        _playerManager->migrateEmail(
                req->attributes()->get<int64_t>("playerId"),
                request["newEmail"].asString(),
                request["code"].asString()
        );
    }, response);
    response.to(callback);
}

void Auth::deactivateEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        auto request = req->attributes()->get<RequestJson>("requestJson");
        _playerManager->deactivateEmail(
                req->attributes()->get<int64_t>("playerId"),
                request["code"].asString()
        );
    }, response);
    response.to(callback);
}