//
// Created by ParticleG on 2022/2/9.
//

#pragma once

#include <drogon/drogon.h>
#include <drogon/WebSocketConnection.h>
#include <types/ErrorNumber.h>
#include <types/JsonValue.h>
#include <types/ResultCode.h>
#include <utils/data.h>

namespace studio26f::helpers {
    class JsonHelper {
    private:
        using HttpCallback = std::function<void(const drogon::HttpResponsePtr &)>;
    public:
        JsonHelper() = default;

        explicit JsonHelper(Json::Value json);

        explicit JsonHelper(const std::string &raw);

        explicit JsonHelper(const drogon::HttpRequestPtr &req);

        explicit JsonHelper(const drogon::HttpResponsePtr &res);

        explicit JsonHelper(drogon::HttpStatusCode statusCode, types::ResultCode resultCode);

        explicit JsonHelper(types::ErrorNumber messageType);

        explicit JsonHelper(const std::string &raw, types::ErrorNumber messageType);

        explicit JsonHelper(int action, types::ErrorNumber messageType);

        virtual ~JsonHelper() = default;

        [[nodiscard]] std::string stringify(const std::string &indentation = "") const;

        Json::Value &ref();

        [[nodiscard]] Json::Value copy() const;

        [[nodiscard]] bool check(const std::string &path, types::JsonValue valueType) const;

        [[nodiscard]] bool check(types::JsonValue valueType) const;

        void require(const std::string &key, types::JsonValue valueType) const;

        void require(types::JsonValue valueType) const;

        void remove(const std::string &key);

        void trim(const std::string &key, types::JsonValue valueType);

        JsonHelper &setResultCode(types::ResultCode code);

        JsonHelper &setResultCode(uint32_t code);

        JsonHelper &setStatusCode(drogon::HttpStatusCode code);

        JsonHelper &setMessageType(types::ErrorNumber type);

        JsonHelper &setAction(int action);

        JsonHelper &setData(Json::Value data);

        JsonHelper &setMessage(const std::string &message);

        JsonHelper &setReason(const std::string &reason);

        JsonHelper &setReason(const std::exception &e);

        void to(const HttpCallback &callback) const;

        void to(const drogon::WebSocketConnectionPtr &connectionPtr) const;

        void operator()(Json::Value &&another);

        const Json::Value &operator[](Json::ArrayIndex index) const;

        const Json::Value &operator[](int index) const;

        const Json::Value &operator[](const char *key) const;

        const Json::Value &operator[](const Json::String &key) const;

        template<internal::JsonTypes T>
        [[nodiscard]] bool check(const std::string &key) const {
            return _value[key].is<T>();
        }

        template<internal::JsonTypes T>
        [[nodiscard]] bool equal(const std::string &key, const T &value) const {
            if (!check<T>(key)) {
                return false;
            }
            return _value[key].as<T>() == value;
        }

        template<internal::JsonTypes T>
        [[nodiscard]] bool notEqual(const std::string &key, const T &value) const {
            if (!check<T>(key)) {
                return false;
            }
            return _value[key].as<T>() != value;
        }

    protected:
        Json::Value _value{Json::objectValue};

    private:
        drogon::HttpStatusCode _statusCode = drogon::k200OK;

        static bool _check(
                const Json::Value &json,
                const types::JsonValue &valueType
        );
    };
}



