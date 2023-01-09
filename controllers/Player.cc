//
// Created by particleg on 2022/9/1.
//

#include <controllers/Player.h>

using namespace drogon;
using namespace std;
using namespace studio26f::api::v1;
using namespace studio26f::helpers;
using namespace studio26f::plugins;
using namespace studio26f::structures;
using namespace studio26f::types;

Player::Player() : _playerManager(app().getPlugin<PlayerManager>()) {}

void Player::getAvatar(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    JsonHelper response(k200OK, ResultCode::Completed);
    handleExceptions([&]() {
        response.setData(_playerManager->getAvatar(
                req->attributes()->get<string>("accessToken"),
                req->attributes()->get<int64_t>("playerId")
        ));
    }, response);
    response.to(callback);
}

void Player::getInfo(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    JsonHelper response(k200OK, ResultCode::Completed);
    handleExceptions([&]() {
        response.setData(_playerManager->getPlayerInfo(
                req->attributes()->get<string>("accessToken"),
                req->attributes()->get<int64_t>("playerId")
        ));
    }, response);
    response.to(callback);
}

void Player::updateInfo(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    JsonHelper response(k200OK, ResultCode::Completed);
    handleExceptions([&]() {
        _playerManager->updatePlayerInfo(
                req->attributes()->get<int64_t>("playerId"),
                req->attributes()->get<JsonHelper>("requestJson")
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