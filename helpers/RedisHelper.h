//
// Created by ParticleG on 2022/2/9.
//

#pragma once

#include <drogon/drogon.h>

namespace studio26f::helpers {
    class RedisHelper : public trantor::NonCopyable {
    public:
        explicit RedisHelper(std::string BaseKey = CMAKE_PROJECT_NAME);

        [[nodiscard]] bool tokenBucket(
                const std::string &key,
                const std::chrono::microseconds &restoreInterval,
                const uint64_t &maxCount
        );

        virtual ~RedisHelper();

    protected:
        void del(const std::string &key);

        bool exists(const std::vector<std::string> &keys);

        void expire(const std::string &key, const std::chrono::seconds &ttl);

        void expire(const std::vector<std::tuple<std::string, std::chrono::seconds>> &params);

        std::string get(const std::string &key);

        void setAdd(const std::string &key, const std::vector<std::string> &values);

        void setAdd(const std::vector<std::pair<std::string, std::vector<std::string>>> &tempKey);

        int64_t setCard(const std::string &key);

        std::vector<std::string> setGetMembers(const std::string &key);

        std::vector<std::vector<std::string>> setGetMembers(const std::vector<std::string> &keys);

        bool setIsMember(const std::string &key, const std::string &value);

        void setRemove(const std::string &key, const std::vector<std::string> &values);

        void setRemove(const std::vector<std::pair<std::string, std::vector<std::string>>> &params);

        void set(const std::string &key, const std::string &value);

        void setEx(
                const std::string &key,
                int ttl,
                const std::string &value
        );

        void setEx(const std::vector<std::tuple<std::string, int, std::string>> &params);

    private:
        std::string _baseKey;
        drogon::nosql::RedisClientPtr _redisClient;
    };
}



