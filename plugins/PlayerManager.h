//
// Created by particleg on 2022/8/29.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <helpers/I18nHelper.h>
#include <helpers/RedisHelper.h>
#include <helpers/RequestJson.h>
#include <helpers/ResponseJson.h>
#include <models/Player.h>

namespace studio26f::plugins {
    class PlayerManager :
            public drogon::Plugin<PlayerManager>,
            public helpers::I18nHelper<PlayerManager>,
            public helpers::RedisHelper {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

        class RedisToken {
        public:
            RedisToken(std::string access, std::string refresh);

            RedisToken(RedisToken &&redisToken) noexcept;

            [[nodiscard]] std::string &access();

            [[nodiscard]] Json::Value parse() const;

        private:
            std::string _accessToken, _refreshToken;
        };

    public:
        PlayerManager();

        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        [[nodiscard]] int64_t getPlayerId(const std::string &accessToken, const std::string &app = "api");

        RedisToken refresh(const std::string &refreshToken);

        void verifyEmail(const std::string &email);

        std::string seedEmail(const std::string &email);

        [[nodiscard]] std::tuple<RedisToken, bool> loginEmailCode(
                const std::string &email,
                const std::string &code
        );

        [[nodiscard]] RedisToken loginEmailPassword(
                const std::string &email,
                const std::string &password
        );

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

        void deactivateEmail(
                int64_t playerId,
                const std::string &code
        );

        [[nodiscard]] std::string getAvatar(
                const std::string &accessToken,
                int64_t playerId
        );

        [[nodiscard]] Json::Value getPlayerInfo(
                const std::string &accessToken,
                int64_t playerId
        );

        void updatePlayerInfo(
                int64_t playerId,
                helpers::RequestJson request
        );

        [[nodiscard]] bool ipLimit(const std::string &ip) const;

        [[nodiscard]] bool loginLimit(const std::string &type, const std::string &key) const;

        [[nodiscard]] bool verifyLimit(const std::string &type, const std::string &key) const;

    private:
        std::chrono::seconds _ipInterval{}, _loginInterval{}, _verifyInterval{},
                _accessExpiration{}, _refreshExpiration{}, _emailExpiration{};
        uint64_t _ipMaxCount{}, _verifyMaxCount{}, _loginMaxCount{};

        drogon::orm::Mapper<drogon_model::studio26f::Player> _playerMapper;

        int64_t _getIdByAccessToken(const std::string &accessToken, const std::string &app = "api");

        void _checkEmailCode(const std::string &email, const std::string &code);

        void _setEmailCode(const std::string &email, const std::string &code);

        RedisToken _generateTokens(const std::string &userId);

        std::string _generateAccessToken(const std::string &userId);

        std::string _generateRefreshToken(const std::string &userId);
    };
}
