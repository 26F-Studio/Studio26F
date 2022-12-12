//
// Created by ParticleG on 2022/2/9.
//

#pragma once

#include <drogon/drogon.h>

namespace studio26f::helpers {
    class RedisHelper {
    public:
        explicit RedisHelper(std::string BaseKey = CMAKE_PROJECT_NAME);

        [[nodiscard]] bool tokenBucket(
                const std::string &key,
                const std::chrono::microseconds &restoreInterval,
                const uint64_t &maxCount
        ) const;

        virtual ~RedisHelper();

    protected:
        template<typename T>
        using KeyPair = std::pair<std::string, T>;

        template<typename T>
        using KeyPairs = std::vector<KeyPair<T>>;

        using SimpleResult = std::pair<bool, std::string>;
        using SimpleResultCb = std::function<void(SimpleResult &&)>;

        using SimpleResults = std::vector<SimpleResult>;
        using SimpleResultsCb = std::function<void(SimpleResults &&)>;

        void del(const std::vector<std::string> &keys, const std::function<void(int64_t)> &callback) const noexcept;

        [[nodiscard]] int64_t del(const std::vector<std::string> &keys) const;

        void exists(const std::vector<std::string> &keys, const std::function<void(bool)> &callback) const noexcept;

        [[nodiscard]] bool exists(const std::vector<std::string> &keys) const;

        void expire(
                const std::string &key,
                const std::chrono::seconds &ttl,
                const std::function<void(bool)> &callback
        ) const noexcept;

        [[nodiscard]] bool expire(const std::string &key, const std::chrono::seconds &ttl) const;

        void expire(
                const KeyPairs<std::chrono::seconds> &params,
                const std::function<void(std::vector<bool> &&)> &callback
        ) const noexcept;

        [[nodiscard]] std::vector<bool> expire(const KeyPairs<std::chrono::seconds> &params) const;

        void get(const std::string &key, const SimpleResultCb &callback) const noexcept;

        [[nodiscard]] std::string get(const std::string &key) const;

        [[nodiscard]] int64_t incrBy(const std::string &key, const int64_t &value = 1) const;

        [[nodiscard]] int64_t decrBy(const std::string &key, const int64_t &value = 1) const;

        void set(const std::string &key, const std::string &value, const SimpleResultCb &callback) const noexcept;

        [[nodiscard]] SimpleResult set(const std::string &key, const std::string &value) const;

        void set(const KeyPairs<std::string> &params, const SimpleResultsCb &callback) const noexcept;

        [[nodiscard]] SimpleResults set(const KeyPairs<std::string> &params) const;

        [[nodiscard]] std::string setEx(const std::string &key, int64_t ttl, const std::string &value) const;

        [[nodiscard]] SimpleResults setEx(
                const std::vector<std::tuple<std::string, int64_t, std::string>> &params
        ) const;

    private:
        std::string _baseKey;
        drogon::nosql::RedisClientPtr _redisClient;
    };
}



