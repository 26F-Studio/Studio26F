//
// Created by particleg on 2022/8/29.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <helpers/RedisHelper.h>
#include <models/Player.h>
#include <structures/ExceptionHandlers.h>
#include <types/Products.h>

namespace studio26f::plugins {
    class PlayerManager :
            public drogon::Plugin<PlayerManager>,
            public structures::HttpRequestHandler<PlayerManager>,
            public helpers::RedisHelper {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        PlayerManager();

        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        void oauth(
                const std::string &accessToken,
                int64_t playerId,
                const std::string &product,
                const std::string &recaptcha,
                trantor::InetAddress address
        );

        int64_t getPlayerIdByAccessToken(const std::string &accessToken);

        bool tryRefresh(std::string &accessToken);

        void verifyEmail(const std::string &email);

        std::string seedEmail(const std::string &email);

        std::tuple<std::string, bool> loginEmailCode(const std::string &email, const std::string &code);

        std::string loginEmailPassword(const std::string &email, const std::string &password);

        void resetEmail(
                const std::string &email,
                const std::string &code,
                const std::string &newPassword
        );

        void migrateEmail(
                int64_t playerId,
                const std::string &newEmail,
                const std::string &code
        );

        void deactivateEmail(int64_t playerId, const std::string &code);

        std::string getAvatar(const std::string &accessToken, int64_t playerId);

        Json::Value getPlayerInfo(const std::string &accessToken, int64_t playerId);

        void updatePlayerInfo(int64_t playerId, helpers::JsonHelper request);

        bool ipLimit(const std::string &ip);

        bool loginLimit(const std::string &type, const std::string &key);

        bool verifyLimit(const std::string &type, const std::string &key);

    private:
        std::string _recaptchaSecret;
        std::chrono::seconds _ipInterval{}, _loginInterval{}, _verifyInterval{},
                _accessExpiration{}, _refreshExpiration{}, _emailExpiration{};
        uint64_t _ipMaxCount{}, _verifyMaxCount{}, _loginMaxCount{};

        std::unordered_map<types::Products, std::string> _productAddressMap;

        drogon::orm::Mapper<drogon_model::studio26f::Player> _playerMapper;

        void _checkEmailCode(const std::string &email, const std::string &code);

        void _setEmailCode(const std::string &email, const std::string &code);

        std::string _generateAccessToken(const std::string &userId);
    };
}
