//
// Created by particleg on 2021/10/2.
//

#include <magic_enum.hpp>
#include <structures/Exceptions.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace studio26f::helpers;
using namespace studio26f::internal;
using namespace studio26f::structures;
using namespace studio26f::types;

BaseException::BaseException(string message) : _message(std::move(message)) {}

char const *BaseException::what() const noexcept { return _message.c_str(); }

ResponseException::ResponseException(
        string message,
        ResultCode resultCode,
        HttpStatusCode statusCode
) : BaseException(std::move(message)), _resultCode(resultCode), _statusCode(statusCode) {}

ResponseException::ResponseException(
        string message,
        const exception &e,
        ResultCode resultCode,
        HttpStatusCode statusCode
) : BaseException(std::move(message)), _reason(e.what()), _resultCode(resultCode), _statusCode(statusCode) {}

JsonHelper ResponseException::toJson() const noexcept {
    return JsonHelper(_statusCode, _resultCode)
            .setMessage(_message)
            .setReason(_reason);
}

MessageException::MessageException(
        string message,
        bool error
) : BaseException(std::move(message)), _error(error) {}

MessageException::MessageException(
        std::string message,
        const exception &e,
        bool error
) : BaseException(std::move(message)), _reason(e.what()), _error(error) {}

JsonHelper MessageException::toJson() const noexcept {
    return JsonHelper(_error ? ErrorNumber::Error : ErrorNumber::Failed)
            .setMessage(_message)
            .setReason(_reason);
}

EmailException::EmailException(
        string message
) : BaseException(std::move(message)) {}

json_exception::InvalidFormat::InvalidFormat(std::string message) :
        BaseException(std::move(message)) {}

json_exception::WrongType::WrongType(JsonValue valueType) :
        BaseException(string(enum_name(valueType))) {}

redis_exception::KeyNotFound::KeyNotFound(string message) :
        BaseException(std::move(message)) {}

[[maybe_unused]] redis_exception::FieldNotFound::FieldNotFound(string message) :
        BaseException(std::move(message)) {}

action_exception::ActionNotFound::ActionNotFound(string message) :
        BaseException(std::move(message)) {}