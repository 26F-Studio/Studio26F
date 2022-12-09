//
// Created by ParticleG on 2022/2/9.
//

#include <helpers/RedisHelper.h>
#include <ranges>
#include <structures/Exceptions.h>
#include <utils/crypto.h>
#include <utils/datetime.h>

using namespace drogon;
using namespace std;
using namespace studio26f::helpers;
using namespace studio26f::structures;
using namespace studio26f::utils;

RedisHelper::RedisHelper(std::string BaseKey) :
        _baseKey(std::move(BaseKey)),
        _redisClient(app().getRedisClient()) {}

RedisHelper::~RedisHelper() {
    _redisClient->closeAll();
    LOG_INFO << "Redis disconnected.";
}

bool RedisHelper::tokenBucket(
        const string &key,
        const chrono::microseconds &restoreInterval,
        const uint64_t &maxCount
) {
    const auto countKey = _baseKey + ":tokenBucket:count:" + key;
    const auto updatedKey = _baseKey + ":tokenBucket:updated:" + key;
    const auto maxTtl = chrono::duration_cast<chrono::seconds>(restoreInterval * maxCount);

    uint64_t countValue;
    try {
        countValue = stoull(get(countKey));
    } catch (...) {
        _redisClient.set(countKey, to_string(maxCount - 1));
        countValue = maxCount;
    }

    bool hasToken = true;
    try {
        const auto lastUpdated = get(updatedKey);
        const auto nowMicroseconds = datetime::toDate().microSecondsSinceEpoch();
        const auto generatedCount =
                (nowMicroseconds -
                 datetime::toDate(lastUpdated).microSecondsSinceEpoch()
                ) / restoreInterval.count() - 1;

        if (generatedCount >= 1) {
            _redisClient.set(updatedKey, datetime::toString(nowMicroseconds));
            _redisClient.incrby(countKey, static_cast<int>(generatedCount) - 1);
            hasToken = true;
        } else if (countValue > 0) {
            _redisClient.decr(countKey);
            hasToken = true;
        } else {
            hasToken = false;
        }
    } catch (...) {
        _redisClient.set(updatedKey, datetime::toString());
        _redisClient.set(countKey, to_string(maxCount - 1));
    }

    // Use sync methods to make sure the operation is completed.
    expire({{countKey,   maxTtl},
            {updatedKey, maxTtl}});
    return hasToken;
}

void RedisHelper::del(const string &key) {
    LOG_TRACE << _redisClient->execCommandSync<int64_t>(
                [](const nosql::RedisResult &result) {
                    return result.asInteger();
                },
                "del %s", key.c_str()
        );
}

bool RedisHelper::exists(const vector<string> &keys) {
    if (keys.empty()) {
        return false;
    }

    stringstream keyStream;
    ranges::copy(keys | views::transform([this](const auto &key) {
        return _baseKey + ":" + key;
    }), ostream_iterator<string>(keyStream, " "));

    try {
        return _redisClient->execCommandSync<bool>(
                [size = keys.size()](const nosql::RedisResult &result) {
                    return result.asInteger() == size;
                },
                "exists %s", keyStream.str().c_str()
        );
    } catch (const nosql::RedisException &err) {
        LOG_ERROR << err.what();
    }
}

void RedisHelper::expire(const string &key, const chrono::seconds &ttl) {
    const auto tempKey = _baseKey + ":" + key;
    try {
        LOG_TRACE << _redisClient->execCommandSync<int64_t>(
                    [](const nosql::RedisResult &result) {
                        return result.asInteger();
                    },
                    "expire %s %d", tempKey.c_str(), ttl.count()
            );
    } catch (const nosql::RedisException &err) {
        LOG_ERROR << err.what();
    }
}

void RedisHelper::expire(const vector<tuple<string, chrono::seconds>> &params) {
    vector<future<reply>> futures;
    futures.reserve(params.size());
    for (const auto &[key, ttl]: params) {
        futures.push_back(_redisClient.expire(_baseKey + ":" + key, static_cast<int>(ttl.count())));
    }
    _redisClient.sync_commit();
    for (auto index = 0; index < futures.size(); ++index) {
        const auto reply = futures[index].get();
        if (reply.is_null()) {
            const auto &[key, _] = params[index];
            throw redis_exception::KeyNotFound(key);
        }
    }
}

string RedisHelper::get(const string &key) {
    const auto tempKey = _baseKey + ":" + key;
    auto future = _redisClient.get(tempKey);
    _redisClient.sync_commit();
    const auto reply = future.get();
    if (reply.is_null()) {
        throw redis_exception::KeyNotFound(tempKey);
    }
    return reply.as_string();
}


void RedisHelper::setAdd(const string &key, const vector<string> &values) {
    _redisClient.sadd(_baseKey + ":" + key, values);
    _redisClient.sync_commit();
}

void RedisHelper::setAdd(const vector<pair<string, vector<string>>> &params) {
    for (const auto &[key, values]: params) {
        _redisClient.sadd(_baseKey + ":" + key, values);
    }
    _redisClient.sync_commit();
}

int64_t RedisHelper::setCard(const string &key) {
    const auto tempKey = _baseKey + ":" + key;
    auto future = _redisClient.scard(tempKey);
    _redisClient.sync_commit();
    const auto reply = future.get();
    if (reply.is_null()) {
        throw redis_exception::KeyNotFound(tempKey);
    }
    return reply.as_integer();
}

vector<string> RedisHelper::setGetMembers(const string &key) {
    const auto tempKey = _baseKey + ":" + key;
    auto future = _redisClient.smembers(tempKey);
    _redisClient.sync_commit();
    const auto reply = future.get();
    if (reply.is_null()) {
        throw redis_exception::KeyNotFound(tempKey);
    }
    const auto &array = reply.as_array();
    vector<string> members;
    transform(array.begin(), array.end(), back_inserter(members), [](const auto &item) {
        return item.as_string();
    });
    return members;
}

vector<vector<string>> RedisHelper::setGetMembers(const vector<string> &keys) {
    vector<future<reply>> futures;
    futures.reserve(keys.size());
    for (const auto &key: keys) {
        futures.push_back(_redisClient.smembers(_baseKey + ":" + key));
    }
    _redisClient.sync_commit();
    vector<vector<string>> result;
    for (auto index = 0; index < futures.size(); ++index) {
        const auto reply = futures[index].get();
        if (reply.is_null()) {
            throw redis_exception::KeyNotFound(keys[index]);
        }
        const auto &array = reply.as_array();
        vector<string> members;
        transform(array.begin(), array.end(), back_inserter(members), [](const auto &item) {
            return item.as_string();
        });
        result.push_back(members);
    }
    return result;
}

bool RedisHelper::setIsMember(const string &key, const string &value) {
    const auto tempKey = _baseKey + ":" + key;
    auto future = _redisClient.sismember(tempKey, value);
    _redisClient.sync_commit();
    const auto reply = future.get();
    if (reply.is_null()) {
        throw redis_exception::KeyNotFound(tempKey);
    }
    return reply.as_integer();
}

void RedisHelper::setRemove(const string &key, const vector<string> &values) {
    _redisClient.srem(_baseKey + ":" + key, values);
    _redisClient.sync_commit();
}

void RedisHelper::setRemove(const vector<pair<string, vector<string>>> &params) {
    for (const auto &[key, values]: params) {
        _redisClient.srem(_baseKey + ":" + key, values);
    }
    _redisClient.sync_commit();
}

void RedisHelper::set(const string &key, const string &value) {
    _redisClient.set(_baseKey + ":" + key, value);
    _redisClient.sync_commit();
}

void RedisHelper::setEx(
        const string &key,
        int ttl,
        const string &value
) {
    _redisClient.setex(_baseKey + ":" + key, ttl, value);
    _redisClient.sync_commit();
}

void RedisHelper::setEx(const vector<tuple<string, int, string>> &params) {
    for (const auto &[key, ttl, value]: params) {
        _redisClient.setex(_baseKey + ":" + key, ttl, value);
    }
    _redisClient.sync_commit();
}
