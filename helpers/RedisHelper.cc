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

RedisHelper::RedisHelper(string BaseKey) :
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
) const {
    const auto countKey = _baseKey + ":tokenBucket:count:" + key;
    const auto updatedKey = _baseKey + ":tokenBucket:updated:" + key;
    const auto maxTtl = chrono::duration_cast<chrono::seconds>(restoreInterval * maxCount);

    uint64_t countValue;
    if (exists({countKey}))
        try {
            countValue = stoull(get(countKey));
        } catch (...) {
            set(countKey, to_string(maxCount - 1));
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
            set(updatedKey, datetime::toString(nowMicroseconds));
            incrBy(countKey, static_cast<int>(generatedCount) - 1);
            hasToken = true;
        } else if (countValue > 0) {
            decrBy(countKey);
            hasToken = true;
        } else {
            hasToken = false;
        }
    } catch (...) {
        set({{updatedKey, datetime::toString()},
             {countKey,   to_string(maxCount - 1)}});
    }

    // Use sync methods to make sure the operation is completed.
    expire({{countKey,   maxTtl},
            {updatedKey, maxTtl}});
    return hasToken;
}

void RedisHelper::del(const vector<string> &keys, const function<void(int64_t)> &callback) const noexcept {
    if (keys.empty()) {
        callback(0);
        return;
    }

    stringstream keyStream;
    ranges::copy(keys | views::transform([this](const auto &key) {
        return _baseKey + ":" + key;
    }), ostream_iterator<string>(keyStream, " "));

    _redisClient->execCommandAsync(
            [&, size = keys.size()](const nosql::RedisResult &result) {
                callback(result.asInteger());
            },
            [&](const exception &err) {
                LOG_ERROR << err.what();
                callback(-1);
            },
            "del %s", keyStream.str().c_str()
    );
}

int64_t RedisHelper::del(const vector<string> &keys) const {
    if (keys.empty()) {
        return 0;
    }

    stringstream keyStream;
    ranges::copy(keys | views::transform([this](const auto &key) {
        return _baseKey + ":" + key;
    }), ostream_iterator<string>(keyStream, " "));

    return _redisClient->execCommandSync<int64_t>(
            [size = keys.size()](const nosql::RedisResult &result) {
                return result.asInteger();
            },
            "del %s", keyStream.str().c_str()
    );
}

void RedisHelper::exists(const vector<string> &keys, const function<void(bool)> &callback) const noexcept {
    if (keys.empty()) {
        callback(false);
        return;
    }

    stringstream keyStream;
    ranges::copy(keys | views::transform([this](const auto &key) {
        return _baseKey + ":" + key;
    }), ostream_iterator<string>(keyStream, " "));

    _redisClient->execCommandAsync(
            [&, size = keys.size()](const nosql::RedisResult &result) {
                callback(result.asInteger() == size);
            },
            [&](const exception &err) {
                LOG_ERROR << err.what();
                callback(false);
            },
            "exists %s", keyStream.str().c_str()
    );
}

bool RedisHelper::exists(const vector<string> &keys) const {
    if (keys.empty()) {
        return false;
    }

    stringstream keyStream;
    ranges::copy(keys | views::transform([this](const auto &key) {
        return _baseKey + ":" + key;
    }), ostream_iterator<string>(keyStream, " "));

    return _redisClient->execCommandSync<bool>(
            [size = keys.size()](const nosql::RedisResult &result) {
                return result.asInteger() == size;
            },
            "exists %s", keyStream.str().c_str()
    );
}

void RedisHelper::expire(
        const string &key,
        const chrono::seconds &ttl,
        const function<void(bool)> &callback
) const noexcept {
    const auto tempKey = _baseKey + ":" + key;
    _redisClient->execCommandAsync(
            [&](const nosql::RedisResult &result) {
                callback(result.asInteger());
            },
            [&](const exception &err) {
                LOG_ERROR << err.what();
                callback(false);
            },
            "expire %s %d", tempKey.c_str(), ttl.count()
    );
}

bool RedisHelper::expire(const string &key, const chrono::seconds &ttl) const {
    const auto tempKey = _baseKey + ":" + key;
    return _redisClient->execCommandSync<bool>(
            [](const nosql::RedisResult &result) {
                return result.asInteger();
            },
            "expire %s %d", tempKey.c_str(), ttl.count()
    );
}

void RedisHelper::expire(
        const KeyPairs <chrono::seconds> &params,
        const function<void(vector<bool> &&)> &callback
) const noexcept {
    const auto transaction = _redisClient->newTransaction();
    for (const auto &[key, ttl]: params) {
        transaction->execCommandAsync(
                [](const nosql::RedisResult &result) {
                    LOG_TRACE << result.getStringForDisplayingWithIndent();
                },
                [](const exception &err) {
                    LOG_ERROR << err.what();
                },
                "expire %s %d", (_baseKey + ":" + key).c_str(), ttl.count()
        );
    }
    transaction->execute(
            [&](const nosql::RedisResult &result) {
                LOG_TRACE << result.getStringForDisplayingWithIndent();
                const auto &resultsArray = result.asArray();
                const auto view = resultsArray | views::transform(
                        [](const auto &item) -> bool { return item.asInteger(); }
                );
                callback({view.begin(), view.end()});
            },
            [&](const exception &err) {
                LOG_ERROR << err.what();
                callback({});
            }
    );
}

vector<bool> RedisHelper::expire(const KeyPairs <chrono::seconds> &params) const {
    const auto transaction = _redisClient->newTransaction();
    for (const auto &[key, ttl]: params) {
        transaction->execCommandAsync(
                [](const nosql::RedisResult &result) {
                    LOG_TRACE << result.getStringForDisplayingWithIndent();
                },
                [](const exception &err) {
                    LOG_ERROR << err.what();
                },
                "expire %s %d", (_baseKey + ":" + key).c_str(), ttl.count()
        );
    }
    promise<vector<bool>> resultsPromise;
    auto resultsFuture = resultsPromise.get_future();
    transaction->execute(
            [&](const nosql::RedisResult &result) {
                LOG_TRACE << result.getStringForDisplayingWithIndent();
                const auto &resultsArray = result.asArray();
                const auto view = resultsArray | views::transform(
                        [](const auto &item) -> bool { return item.asInteger(); }
                );
                resultsPromise.set_value({view.begin(), view.end()});
            },
            [&](const exception &err) {
                LOG_ERROR << err.what();
                resultsPromise.set_value({});
            }
    );
    return resultsFuture.get();
}

void RedisHelper::get(const string &key, const RedisHelper::SimpleResultCb &callback) const noexcept {
    const auto tempKey = _baseKey + ":" + key;
    _redisClient->execCommandAsync(
            [&](const nosql::RedisResult &result) {
                if (result.isNil()) {
                    callback({false, {}});
                } else {
                    callback({true, result.asString()});
                }
            },
            [&](const exception &err) {
                LOG_ERROR << err.what();
                callback({false, err.what()});
            },
            "get %s", tempKey.c_str()
    );
}

string RedisHelper::get(const string &key) const {
    const auto tempKey = _baseKey + ":" + key;
    return _redisClient->execCommandSync<string>(
            [=](const nosql::RedisResult &result) {
                if (result.isNil()) {
                    throw redis_exception::KeyNotFound(tempKey);
                }
                return result.asString();
            },
            "get %s", tempKey.c_str()
    );
}

int64_t RedisHelper::incrBy(const string &key, const int64_t &value) const {
    const auto tempKey = _baseKey + ":" + key;
    return _redisClient->execCommandSync<int64_t>(
            [](const nosql::RedisResult &result) {
                return result.asInteger();
            },
            "incrBy %s %lld", tempKey.c_str(), value
    );
}

int64_t RedisHelper::decrBy(const string &key, const int64_t &value) const {
    const auto tempKey = _baseKey + ":" + key;
    return _redisClient->execCommandSync<int64_t>(
            [](const nosql::RedisResult &result) {
                return result.asInteger();
            },
            "decrBy %s %lld", tempKey.c_str(), value
    );
}

void RedisHelper::set(const string &key, const string &value, const SimpleResultCb &callback) const noexcept {
    const auto tempKey = _baseKey + ":" + key;
    _redisClient->execCommandAsync(
            [&](const nosql::RedisResult &result) {
                const auto resultString = result.asString();
                LOG_TRACE << result.getStringForDisplayingWithIndent();
                callback({true, result.asString()});
            },
            [&](const exception &err) {
                LOG_ERROR << err.what();
                callback({false, err.what()});
            },
            "set %s %s", tempKey.c_str(), value.c_str()
    );
}

RedisHelper::SimpleResult RedisHelper::set(const string &key, const string &value) const noexcept(false) {
    const auto tempKey = _baseKey + ":" + key;
    return _redisClient->execCommandSync<SimpleResult>(
            [&](const nosql::RedisResult &result) -> SimpleResult {
                const auto resultString = result.asString();
                LOG_TRACE << result.getStringForDisplayingWithIndent();
                return {true, result.asString()};
            },
            "set %s %s", tempKey.c_str(), value.c_str()
    );
}

void RedisHelper::set(const KeyPairs <string> &params, const SimpleResultsCb &callback) const noexcept {
    const auto transaction = _redisClient->newTransaction();
    for (const auto &[key, value]: params) {
        const auto tempKey = _baseKey + ":" + key;
        transaction->execCommandAsync(
                [&](const nosql::RedisResult &result) {
                    LOG_TRACE << result.getStringForDisplayingWithIndent();
                },
                [&](const exception &err) {
                    LOG_ERROR << err.what();
                },
                "set %s %s", tempKey.c_str(), value.c_str()
        );
    }
    transaction->execute(
            [&](const nosql::RedisResult &result) {
                LOG_TRACE << result.getStringForDisplayingWithIndent();
                const auto &resultsArray = result.asArray();
                const auto view = resultsArray | views::transform(
                        [&result](const auto &item) -> SimpleResult {
                            return {true, result.asString()};
                        }
                );
                callback({view.begin(), view.end()});
            },
            [&](const exception &err) {
                LOG_ERROR << err.what();
                callback({{false, err.what()}});
            }
    );
}

RedisHelper::SimpleResults RedisHelper::set(const KeyPairs <string> &params) const noexcept(false) {
    const auto transaction = _redisClient->newTransaction();
    for (const auto &[key, value]: params) {
        const auto tempKey = _baseKey + ":" + key;
        transaction->execCommandAsync(
                [&](const nosql::RedisResult &result) {
                    LOG_TRACE << result.getStringForDisplayingWithIndent();
                },
                [&](const exception &err) {
                    LOG_ERROR << err.what();
                },
                "set %s %s", tempKey.c_str(), value.c_str()
        );
    }
    promise<SimpleResults> resultsPromise;
    auto resultsFuture = resultsPromise.get_future();
    transaction->execute(
            [&](const nosql::RedisResult &result) {
                LOG_TRACE << result.getStringForDisplayingWithIndent();
                const auto &resultsArray = result.asArray();
                const auto view = resultsArray | views::transform(
                        [&result](const auto &item) -> SimpleResult {
                            return {true, result.asString()};
                        }
                );
                resultsPromise.set_value({view.begin(), view.end()});
            },
            [&](const exception &err) {
                LOG_ERROR << err.what();
                resultsPromise.set_value({{false, err.what()}});
            }
    );
    return resultsFuture.get();
}

string RedisHelper::setEx(const string &key, int64_t ttl, const string &value) const {
    const auto tempKey = _baseKey + ":" + key;
    return _redisClient->execCommandSync<string>(
            [](const nosql::RedisResult &result) {
                return result.asString();
            },
            "setEx %s %lld %s", tempKey.c_str(), ttl, value.c_str()
    );
}

RedisHelper::SimpleResults RedisHelper::setEx(const vector<tuple<string, int64_t, string>> &params) const {
    const auto transaction = _redisClient->newTransaction();
    for (const auto &[key, ttl, value]: params) {
        const auto tempKey = _baseKey + ":" + key;
        transaction->execCommandAsync(
                [&](const nosql::RedisResult &result) {
                    LOG_TRACE << result.getStringForDisplayingWithIndent();
                },
                [&](const exception &err) {
                    LOG_ERROR << err.what();
                },
                "setEx %s %lld %s", tempKey.c_str(), ttl, value.c_str()
        );
    }
    promise<SimpleResults> resultsPromise;
    auto resultsFuture = resultsPromise.get_future();
    transaction->execute(
            [&](const nosql::RedisResult &result) {
                LOG_TRACE << result.getStringForDisplayingWithIndent();
                const auto &resultsArray = result.asArray();
                const auto view = resultsArray | views::transform(
                        [&result](const auto &item) -> SimpleResult {
                            return {true, result.asString()};
                        }
                );
                resultsPromise.set_value({view.begin(), view.end()});
            },
            [&](const exception &err) {
                LOG_ERROR << err.what();
                resultsPromise.set_value({{false, err.what()}});
            }
    );
    return resultsFuture.get();
}