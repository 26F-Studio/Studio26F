//
// Created by particleg on 2022/8/29.
//

#include <ranges>

#include <drogon/drogon.h>
#include <magic_enum.hpp>
#include <plugins/EmailManager.h>
#include <plugins/PlayerManager.h>
#include <structures/ExceptionHandlers.h>
#include <structures/Exceptions.h>
#include <types/Permission.h>
#include <utils/crypto.h>
#include <utils/data.h>
#include <utils/io.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace studio26f::helpers;
using namespace studio26f::plugins;
using namespace studio26f::structures;
using namespace studio26f::types;
using namespace studio26f::utils;

using Player = drogon_model::studio26f::Player;

PlayerManager::PlayerManager() : RedisHelper(CMAKE_PROJECT_NAME), _playerMapper(app().getDbClient()) {}

void PlayerManager::initAndStart(const Json::Value& config) {
    if (!(
        config["expirations"]["access"].isUInt64() &&
        config["expirations"]["refresh"].isUInt64() &&
        config["expirations"]["email"].isUInt64()
    )) {
        LOG_ERROR << R"("Invalid expiration config")";
        abort();
    }
    _accessExpiration = chrono::minutes(config["expirations"]["access"].asUInt64());
    _refreshExpiration = chrono::minutes(config["expirations"]["refresh"].asUInt64());
    _emailExpiration = chrono::minutes(config["expirations"]["email"].asUInt64());

    if (!(
        config["tokenBuckets"]["ip"]["interval"].isUInt64() &&
        config["tokenBuckets"]["ip"]["maxCount"].isUInt64() &&
        config["tokenBuckets"]["login"]["interval"].isUInt64() &&
        config["tokenBuckets"]["login"]["maxCount"].isUInt64() &&
        config["tokenBuckets"]["verify"]["interval"].isUInt64() &&
        config["tokenBuckets"]["verify"]["maxCount"].isUInt64()
    )) {
        LOG_ERROR << R"(Invalid tokenBucket config)";
        abort();
    }
    _ipInterval = chrono::seconds(config["tokenBuckets"]["ip"]["interval"].asUInt64());
    _ipMaxCount = config["tokenBuckets"]["ip"]["maxCount"].asUInt64();
    _loginInterval = chrono::seconds(config["tokenBuckets"]["login"]["interval"].asUInt64());
    _loginMaxCount = config["tokenBuckets"]["login"]["maxCount"].asUInt64();
    _verifyInterval = chrono::seconds(config["tokenBuckets"]["verify"]["interval"].asUInt64());
    _verifyMaxCount = config["tokenBuckets"]["verify"]["maxCount"].asUInt64();

    if (!(
        config["oauth"]["secret"].isString() &&
        config["oauth"]["products"]["quatrack"]["host"].isString() &&
        config["oauth"]["products"]["quatrack"]["path"].isString() &&
        config["oauth"]["products"]["techmino"]["host"].isString() &&
        config["oauth"]["products"]["techmino"]["path"].isString() &&
        config["oauth"]["products"]["techminoGalaxy"]["host"].isString() &&
        config["oauth"]["products"]["techminoGalaxy"]["path"].isString()
    )) {
        LOG_ERROR << R"(Invalid recaptcha config)";
        abort();
    }
    _recaptchaSecret = config["oauth"]["secret"].asString();
    _productAddressMap[Products::quatrack] = {
        config["oauth"]["products"]["quatrack"]["host"].asString(),
        config["oauth"]["products"]["quatrack"]["path"].asString(),
        config["oauth"]["products"]["quatrack"]["secret"].asString(),
    };
    _productAddressMap[Products::techmino] = {
        config["oauth"]["products"]["techmino"]["host"].asString(),
        config["oauth"]["products"]["techmino"]["path"].asString(),
        config["oauth"]["products"]["techmino"]["secret"].asString(),
    };
    _productAddressMap[Products::techminoGalaxy] = {
        config["oauth"]["products"]["techminoGalaxy"]["host"].asString(),
        config["oauth"]["products"]["techminoGalaxy"]["path"].asString(),
        config["oauth"]["products"]["techminoGalaxy"]["secret"].asString(),
    };
    LOG_INFO << "PlayerManager loaded.";
}

void PlayerManager::shutdown() {
    LOG_INFO << "PlayerManager shutdown.";
}

string PlayerManager::oauth(
    const string& recaptcha,
    const trantor::InetAddress& address,
    const string& product,
    int64_t playerId
) {
    const auto productOptional = enum_cast<Products>(product);
    if (!productOptional.has_value()) {
        throw ResponseException(
            i18n("invalidArguments"),
            ResultCode::InvalidArguments,
            k400BadRequest
        );
    } {
        const auto [host, path, secret] = _productAddressMap[productOptional.value()];
        setClient(host);
        Json::Value body;
        body["playerId"] = playerId;
        const auto response = request(
            Post,
            format("{}/auth/oauth/{}", path, secret),
            {},
            {
                {"code", JsonValue::UInt64}
            },
            {},
            body,
            false
        );
        if (response.notEqual("code", enum_integer(ResultCode::Completed))) {
            throw ResponseException(
                response["message"].isString() ? response["message"].asString() : i18n("invalidArguments"),
                enum_cast<ResultCode>(response["code"].asUInt64()).value_or(ResultCode::Unknown),
                k403Forbidden
            );
        }
        if (!response.check("data", JsonValue::String)) {
            throw ResponseException(
                i18n("networkError"),
                ResultCode::NetworkError,
                drogon::k503ServiceUnavailable
            );
        }
        return response["data"].asString();
    }
}

int64_t PlayerManager::getPlayerIdByAccessToken(const string& accessToken) {
    try {
        LOG_DEBUG << "accessId: " << get(data::join({"auth", "access-id", accessToken}, ':'));
        return stoll(get(data::join({"auth", "access-id", accessToken}, ':')));
    } catch (const redis_exception::KeyNotFound& e) {
        throw ResponseException(
            i18n("invalidAccessToken"),
            e,
            ResultCode::NotAcceptable,
            k401Unauthorized
        );
    }
}

bool PlayerManager::tryRefresh(string& accessToken) {
    const auto ttl = chrono::milliseconds(pTtl(data::join({"auth", "access-id", accessToken}, ':')));
    LOG_DEBUG << "ttl: " << ttl.count() << ", _refreshExpiration: " << _refreshExpiration.count();
    if (ttl.count() == -2) {
        throw ResponseException(
            i18n("invalidAccessToken"),
            ResultCode::NotAcceptable,
            k401Unauthorized
        );
    }
    if (ttl < _refreshExpiration) {
        LOG_DEBUG << "refreshing";
        const auto playerId = getPlayerIdByAccessToken(accessToken);
        NO_EXCEPTION(
            del(data::join({"auth", "access-id", accessToken}, ':'));
        )
        accessToken = _generateAccessToken(to_string(playerId));
        return true;
    }
    return false;
}

void PlayerManager::verifyEmail(const string& email) {
    const auto code = data::randomString(8);
    _setEmailCode(email, code);
    // TODO: Make I18N emails
    auto mailContent = io::getFileContent("./verifyEmail.html");
    drogon::utils::replaceAll(
        mailContent,
        "{{VERIFY_CODE}}",
        code
    );
    mailContent = regex_replace(mailContent, regex{R"((\s*[\r\n]+\s*|\s+))"}, " ");
    app().getPlugin<EmailManager>()->smtp(
        email,
        "[26F Studio] Verification Code/验证码",
        mailContent
    );
}

string PlayerManager::seedEmail(const string& email) {
    try {
        const auto player = _playerMapper.findOne(orm::Criteria(
            Player::Cols::_email,
            orm::CompareOperator::EQ,
            email
        ));
        return crypto::blake2B(to_string(player.getValueOfId()));
    } catch (const orm::UnexpectedRows&) {
        return crypto::blake2B(to_string(-1));
    }
}

tuple<string, bool> PlayerManager::loginEmailCode(const string& email, const string& code) {
    _checkEmailCode(email, code);

    Player player;
    if (_playerMapper.count(orm::Criteria(
            Player::Cols::_email,
            orm::CompareOperator::EQ,
            email
        )) == 0) {
        player.setEmail(email);
        _playerMapper.insert(player);

        _setEmailCode(email, code);
    } else {
        player = _playerMapper.findOne(orm::Criteria(
            Player::Cols::_email,
            orm::CompareOperator::EQ,
            email
        ));

        if (player.getPasswordHash() == nullptr) {
            _setEmailCode(email, code);
        }
    }

    return {
        _generateAccessToken(to_string(player.getValueOfId())),
        player.getPasswordHash() == nullptr
    };
}

string PlayerManager::loginEmailPassword(const string& email, const string& password) {
    try {
        auto player = _playerMapper.findOne(orm::Criteria(
            Player::Cols::_email,
            orm::CompareOperator::EQ,
            email
        ));

        if (player.getPasswordHash() == nullptr) {
            throw ResponseException(
                i18n("noPassword"),
                ResultCode::NullValue,
                k403Forbidden
            );
        }

        const auto id = to_string(player.getValueOfId());

        if (player.getValueOfPasswordHash() != crypto::blake2B(password + crypto::blake2B(id))) {
            throw orm::UnexpectedRows("Incorrect password");
        }

        return _generateAccessToken(id);
    } catch (const orm::UnexpectedRows&) {
        LOG_DEBUG << "invalidEmailPass: " << email;
        throw ResponseException(
            i18n("invalidEmailPass"),
            ResultCode::NotAcceptable,
            k403Forbidden
        );
    }
}

void PlayerManager::resetEmail(
    const string& email,
    const string& code,
    const string& newPassword
) {
    _checkEmailCode(email, code);

    try {
        auto player = _playerMapper.findOne(orm::Criteria(
            Player::Cols::_email,
            orm::CompareOperator::EQ,
            email
        ));
        player.setPasswordHash(crypto::blake2B(
            newPassword + crypto::blake2B(to_string(player.getValueOfId()))
        ));
        _playerMapper.update(player);
    } catch (const orm::UnexpectedRows&) {
        LOG_DEBUG << "playerNotFound: " << email;
        throw ResponseException(
            i18n("playerNotFound"),
            ResultCode::NotFound,
            k404NotFound
        );
    }
}

void PlayerManager::migrateEmail(
    const int64_t playerId,
    const string& newEmail,
    const string& code
) {
    _checkEmailCode(newEmail, code);

    try {
        auto player = _playerMapper.findByPrimaryKey(playerId);
        if (player.getValueOfEmail() == newEmail) {
            return;
        }
        if (_playerMapper.count(orm::Criteria(
            Player::Cols::_email,
            orm::CompareOperator::EQ,
            newEmail
        ))) {
            throw ResponseException(
                i18n("emailExists"),
                ResultCode::Conflict,
                k409Conflict
            );
        }
        player.setEmail(newEmail);
        _playerMapper.update(player);
    } catch (const redis_exception::KeyNotFound& e) {
        throw ResponseException(
            i18n("invalidAccessToken"),
            e,
            ResultCode::NotAcceptable,
            k401Unauthorized
        );
    }
}

void PlayerManager::deactivateEmail(
    const int64_t playerId,
    const string& code
) {
    try {
        auto player = _playerMapper.findByPrimaryKey(playerId);
        _checkEmailCode(player.getValueOfEmail(), code);

        player.setPermission(string{enum_name(Permission::Banned)});
        _playerMapper.update(player);
    } catch (const redis_exception::KeyNotFound& e) {
        throw ResponseException(
            i18n("invalidAccessToken"),
            e,
            ResultCode::NotAcceptable,
            k401Unauthorized
        );
    } {
        for (const auto& [host, path, secret]: _productAddressMap | views::values) {
            setClient(host);
            Json::Value body;
            body["playerId"] = playerId;
            if (const auto response = request(
                Post,
                format("{}/auth/deactivate/{}", path, secret),
                {},
                {
                    {"code", JsonValue::UInt64}
                },
                {},
                body,
                false
            ); response.notEqual("code", enum_integer(ResultCode::Completed))) {
                throw ResponseException(
                    response["message"].isString() ? response["message"].asString() : i18n("invalidArguments"),
                    enum_cast<ResultCode>(response["code"].asUInt64()).value_or(ResultCode::Unknown),
                    k403Forbidden
                );
            }
        }
    }
}

string PlayerManager::getAvatar(const string& accessToken, int64_t playerId) {
    int64_t targetId = playerId;
    NO_EXCEPTION(
        targetId = getPlayerIdByAccessToken(accessToken);
    )
    try {
        auto player = _playerMapper.findOne(orm::Criteria(
            Player::Cols::_id,
            orm::CompareOperator::EQ,
            targetId
        ));
        return player.getValueOfAvatar();
    } catch (const orm::UnexpectedRows&) {
        LOG_DEBUG << "playerNotFound: " << targetId;
        throw ResponseException(
            i18n("playerNotFound"),
            ResultCode::NotFound,
            k404NotFound
        );
    }
}

Json::Value PlayerManager::getPlayerInfo(const string& accessToken, int64_t playerId) {
    int64_t targetId = playerId;
    NO_EXCEPTION(
        targetId = getPlayerIdByAccessToken(accessToken);
    )
    try {
        auto player = _playerMapper.findOne(orm::Criteria(
            Player::Cols::_id,
            orm::CompareOperator::EQ,
            targetId
        )).toJson();
        player.removeMember("password_hash");
        player.removeMember("avatar");
        if (playerId > 0) {
            player.removeMember("email");
            player.removeMember("phone");
        }
        return player;
    } catch (const orm::UnexpectedRows&) {
        throw ResponseException(
            i18n("playerNotFound"),
            ResultCode::NotFound,
            k404NotFound
        );
    }
}

void PlayerManager::updatePlayerInfo(int64_t playerId, JsonHelper request) {
    auto player = _playerMapper.findByPrimaryKey(playerId);
    if (request.check("avatar", JsonValue::String)) {
        player.setAvatarHash(crypto::blake2B(request["avatar"].asString()));
    }
    player.updateByJson(request.ref());
    _playerMapper.update(player);
}

bool PlayerManager::ipLimit(const string& ip) {
    return tokenBucket(
        data::join({"ip", ip}, ':'),
        _ipInterval,
        _ipMaxCount
    );
}

bool PlayerManager::loginLimit(const string& type, const string& key) {
    return tokenBucket(
        data::join({"code", type, key}, ':'),
        _loginInterval,
        _loginMaxCount
    );
}

bool PlayerManager::verifyLimit(const string& type, const string& key) {
    return tokenBucket(
        data::join({"verify", type, key}, ':'),
        _verifyInterval,
        _verifyMaxCount
    );
}

void PlayerManager::_checkEmailCode(const string& email, const string& code) {
    const auto key = data::join({"auth", "code", "email", email}, ':');
    if (exists({key})) {
        if (get(key) != code) {
            throw ResponseException(
                i18n("invalidCode"),
                ResultCode::NotAcceptable,
                k403Forbidden
            );
        }
        del({data::join({"auth", "code", "email", email}, ':')});
    } else {
        throw ResponseException(
            i18n("invalidEmail"),
            ResultCode::NotFound,
            k404NotFound
        );
    }
}

void PlayerManager::_setEmailCode(const string& email, const string& code) {
    setPx(
        data::join({"auth", "code", "email", email}, ':'),
        code,
        _emailExpiration
    );
}

string PlayerManager::_generateAccessToken(const string& playerId) {
    auto accessToken = crypto::blake2B(drogon::utils::getUuid());
    setPx(
        data::join({"auth", "access-id", accessToken}, ':'),
        playerId,
        _accessExpiration + _refreshExpiration
    );
    return accessToken;
}
