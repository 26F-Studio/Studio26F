//
// Created by ParticleG on 2022/2/9.
//

#pragma once

#include <drogon/drogon.h>

namespace studio26f::helpers {
    class RedisHelper {
    public:
        explicit RedisHelper(std::string BaseKey);

        virtual ~RedisHelper();

    protected:
        template<typename T>
        using KeyPair = std::pair<std::string, T>;

        template<typename T>
        using KeyPairs = std::vector<KeyPair<T>>;

        using SimpleResult = std::pair<bool, std::string>;

        using SimpleResults = std::vector<SimpleResult>;

        bool tokenBucket(
                const std::string &key,
                const std::chrono::microseconds &restoreInterval,
                const uint64_t &maxCount
        );

        int64_t del(const std::vector<std::string> &keys);

        bool exists(const std::vector<std::string> &keys);

        [[maybe_unused]] bool expire(const std::string &key, const std::chrono::seconds &ttl);

        [[maybe_unused]] std::vector<bool> expire(const KeyPairs<std::chrono::seconds> &params);

        std::vector<bool> pExpire(const KeyPairs<std::chrono::milliseconds> &params);

        std::string get(const std::string &key);

        int64_t incrBy(const std::string &key, const int64_t &value = 1);

        int64_t decrBy(const std::string &key, const int64_t &value = 1);

        SimpleResult set(const std::string &key, const std::string &value);

        SimpleResults set(const std::vector<std::tuple<std::string, std::string>> &params);

        SimpleResult setPx(const std::string &key, const std::string &value, const std::chrono::milliseconds &ttl);

        SimpleResults setPx(const std::vector<std::tuple<std::string, std::string, std::chrono::milliseconds>> &params);

        [[maybe_unused]] std::chrono::seconds ttl(const std::string &key);

        [[maybe_unused]] std::chrono::milliseconds pTtl(const std::string &key);

    private:
        std::string _baseKey;
        drogon::nosql::RedisClientPtr _redisClient;
    };
}



