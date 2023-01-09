//
// Created by ParticleG on 2022/2/9.
//

#include <magic_enum.hpp>
#include <helpers/JsonHelper.h>
#include <structures/Exceptions.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace studio26f::helpers;
using namespace studio26f::structures;
using namespace studio26f::types;

JsonHelper::JsonHelper(Json::Value json) : _value(std::move(json)) {}

JsonHelper::JsonHelper(const string &raw) { stringstream(raw) >> _value; }

JsonHelper::JsonHelper(const HttpRequestPtr &req) {
    auto object = req->getJsonObject();
    if (!object) {
        throw json_exception::InvalidFormat(req->getJsonError());
    }
    _value = std::move(*object);
}

JsonHelper::JsonHelper(const HttpResponsePtr &res) {
    auto object = res->getJsonObject();
    if (!object) {
        throw json_exception::InvalidFormat(res->getJsonError());
    }
    _value = std::move(*object);
}

JsonHelper::JsonHelper(
        HttpStatusCode statusCode,
        ResultCode resultCode
) : _statusCode(statusCode) { setResultCode(resultCode); }

JsonHelper::JsonHelper(ErrorNumber messageType) { setMessageType(messageType); }

JsonHelper::JsonHelper(const string &raw, ErrorNumber messageType) :
        JsonHelper(raw) { setMessageType(messageType); }

JsonHelper::JsonHelper(int action, ErrorNumber messageType) :
        JsonHelper(messageType) { setAction(action); }

string JsonHelper::stringify(const string &indentation) const {
    Json::StreamWriterBuilder writerBuilder;
    writerBuilder.settings_["indentation"] = indentation;
    std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
    std::ostringstream oss;
    jsonWriter->write(_value, &oss);
    return oss.str();
}

Json::Value &JsonHelper::ref() { return _value; }

Json::Value JsonHelper::copy() const { return _value; }

bool JsonHelper::check(const string &path, JsonValue valueType) const {
    auto resultPtr = &_value;
    for (const auto &keyString: drogon::utils::splitString(path, ".")) {
        if (resultPtr->isNull()) {
            return valueType == JsonValue::Null;
        }
        resultPtr = &(*resultPtr)[keyString];
    }
    return _check(*resultPtr, valueType);
}

bool JsonHelper::check(JsonValue valueType) const { return _check(_value, valueType); }

void JsonHelper::require(const string &key, JsonValue valueType) const {
    if (!check(key, valueType)) {
        throw json_exception::WrongType(valueType);
    }
}

void JsonHelper::require(JsonValue valueType) const {
    if (!check(valueType)) {
        throw json_exception::WrongType(valueType);
    }
}

void JsonHelper::remove(const string &key) { _value.removeMember(key); }

void JsonHelper::trim(const string &key, JsonValue valueType) {
    if (!check(key, valueType)) {
        remove(key);
    }
}

JsonHelper &JsonHelper::setResultCode(ResultCode code) {
    setResultCode(enum_integer(code));
    return *this;
}

JsonHelper &JsonHelper::setResultCode(uint32_t code) {
    _value["code"] = code;
    return *this;
}

JsonHelper &JsonHelper::setStatusCode(drogon::HttpStatusCode code) {
    _statusCode = code;
    return *this;
}

JsonHelper &JsonHelper::setMessageType(ErrorNumber type) {
    _value["errno"] = enum_integer(type);
    return *this;
}

JsonHelper &JsonHelper::setAction(int action) {
    _value["action"] = action;
    return *this;
}

JsonHelper &JsonHelper::setData(Json::Value data) {
    if (!data.empty()) {
        _value["data"] = std::move(data);
    } else {
        _value.removeMember("data");
    }
    return *this;
}

JsonHelper &JsonHelper::setMessage(const string &message) {
    if (!message.empty()) {
        _value["message"] = message;
    } else {
        _value.removeMember("message");
    }
    return *this;
}

JsonHelper &JsonHelper::setReason(const string &reason) {
    if (!reason.empty()) {
        _value["reason"] = reason;
    } else {
        _value.removeMember("reason");
    }
    return *this;
}

JsonHelper &JsonHelper::setReason(const exception &e) {
    setReason(e.what());
    return *this;
}

void JsonHelper::to(const JsonHelper::HttpCallback &callback) const {
    auto httpJsonResponse = HttpResponse::newHttpJsonResponse(_value);
    httpJsonResponse->setStatusCode(_statusCode);
    callback(httpJsonResponse);
}

void JsonHelper::to(const WebSocketConnectionPtr &connectionPtr) const {
    if (connectionPtr->connected()) {
        if (enum_cast<ErrorNumber>(_value["errno"].asInt()).value_or(ErrorNumber::Success) == ErrorNumber::Error) {
            connectionPtr->shutdown(CloseCode::kViolation, stringify());
        } else {
            connectionPtr->send(stringify());
        }
    }
}

void JsonHelper::operator()(Json::Value &&another) { _value = std::move(another); }

const Json::Value &JsonHelper::operator[](Json::ArrayIndex index) const { return _value[index]; }

const Json::Value &JsonHelper::operator[](int index) const { return _value[index]; }

const Json::Value &JsonHelper::operator[](const char *key) const { return _value[key]; }

const Json::Value &JsonHelper::operator[](const Json::String &key) const { return _value[key]; }

bool JsonHelper::_check(
        const Json::Value &json,
        const JsonValue &valueType
) {
    switch (valueType) {
        case JsonValue::Null:
            return json.isNull();
        case JsonValue::Object:
            return json.isObject();
        case JsonValue::Array:
            return json.isArray();
        case JsonValue::Bool:
            return json.isBool();
        case JsonValue::UInt:
            return json.isUInt();
        case JsonValue::UInt64:
            return json.isUInt64();
        case JsonValue::Int:
            return json.isInt();
        case JsonValue::Int64:
            return json.isInt64();
        case JsonValue::Double:
            return json.isDouble();
        case JsonValue::String:
            return json.isString();
    }
    return false;
}
