//
// Created by ParticleG on 2022/2/8.
//

#pragma once

#include <drogon/drogon.h>
#include <magic_enum.hpp>
#include <helpers/I18nHelper.h>
#include <helpers/JsonHelper.h>
#include <structures/Exceptions.h>

namespace studio26f::structures {

#define NO_EXCEPTION(expressions) try {expressions} catch (...) {}

    template<class T>
    class RequestJsonHandler : public helpers::I18nHelper<T> {
    public:
        void handleExceptions(
                const std::function<void()> &mainFunction,
                drogon::FilterCallback &failedCb
        ) const {
            using namespace drogon;
            using namespace std;
            using namespace studio26f::helpers;
            using namespace studio26f::structures;
            using namespace studio26f::types;

            try {
                mainFunction();
            } catch (const json_exception::InvalidFormat &e) {
                JsonHelper(k400BadRequest, ResultCode::InvalidFormat)
                        .setMessage(I18nHelper<T>::i18n("invalidFormat"))
                        .setReason(e)
                        .to(failedCb);
            } catch (const json_exception::WrongType &e) {
                JsonHelper(k400BadRequest, ResultCode::InvalidArguments)
                        .setMessage(I18nHelper<T>::i18n("invalidArguments"))
                        .setReason(e)
                        .to(failedCb);
            } catch (const exception &e) {
                LOG_ERROR << e.what();
                JsonHelper(k500InternalServerError, ResultCode::InternalError)
                        .setMessage(I18nHelper<T>::i18n("internalError"))
                        .to(failedCb);
            }
        }

        ~RequestJsonHandler() override = default;
    };

    template<class T>
    class ResponseJsonHandler : public helpers::I18nHelper<T> {
    public:
        void handleExceptions(
                const std::function<void()> &mainFunction,
                helpers::JsonHelper &response
        ) const {
            using namespace drogon;
            using namespace std;
            using namespace studio26f::helpers;
            using namespace studio26f::structures;
            using namespace studio26f::types;

            try {
                mainFunction();
            } catch (const ResponseException &e) {
                response = e.toJson();
            } catch (const orm::DrogonDbException &e) {
                LOG_ERROR << e.base().what();
                response.setStatusCode(k500InternalServerError)
                        .setResultCode(ResultCode::DatabaseError)
                        .setMessage(I18nHelper<T>::i18n("databaseError"));
            } catch (const exception &e) {
                LOG_ERROR << e.what();
                response.setStatusCode(k500InternalServerError)
                        .setResultCode(ResultCode::InternalError)
                        .setMessage(I18nHelper<T>::i18n("internalError"));
            }
        }

        virtual ~ResponseJsonHandler() = default;
    };

    template<class T>
    class MessageJsonHandler : public helpers::I18nHelper<T> {
    public:
        void handleExceptions(
                const std::function<void()> &mainFunction,
                int action,
                const drogon::WebSocketConnectionPtr &wsConnPtr
        ) const {
            using namespace drogon;
            using namespace std;
            using namespace studio26f::helpers;
            using namespace studio26f::structures;
            using namespace studio26f::types;

            try {
                mainFunction();
            } catch (const MessageException &e) {
                e.toJson().setAction(action).to(wsConnPtr);
            } catch (const orm::DrogonDbException &e) {
                LOG_ERROR << e.base().what();
                JsonHelper(action, ErrorNumber::Error)
                        .setMessage(I18nHelper<T>::i18n("databaseError"))
                        .to(wsConnPtr);
            } catch (const exception &e) {
                LOG_ERROR << e.what();
                JsonHelper(action, ErrorNumber::Error)
                        .setMessage(I18nHelper<T>::i18n("internalError"))
                        .to(wsConnPtr);
            }
        }

        virtual ~MessageJsonHandler() = default;
    };

    template<class T>
    class HttpRequestHandler : public helpers::I18nHelper<T> {
    public:
        HttpRequestHandler() = default;

        [[maybe_unused]] HttpRequestHandler(const std::string &ip, uint16_t port) :
                httpClient(drogon::HttpClient::newHttpClient(ip, port)),
                httpsClient(drogon::HttpClient::newHttpClient(ip, port, true)) {}

        [[nodiscard]] helpers::JsonHelper request(
                drogon::HttpMethod method,
                const std::string &path,
                const std::vector<std::pair<std::string, std::string>> &params = {},
                const std::vector<std::pair<std::string, types::JsonValue>> &constraints = {},
                const std::vector<std::pair<std::string, std::string>> &headers = {},
                const Json::Value &body = Json::nullValue,
                bool useSsl = true
        ) const {
            using namespace drogon;
            using namespace helpers;
            using namespace magic_enum;
            using namespace std;
            using namespace structures;
            using namespace types;

            auto req = body.isNull() ? HttpRequest::newHttpRequest() : HttpRequest::newHttpJsonRequest(body);
            req->setMethod(method);
            req->setPath(path);
            for (const auto &param: params) {
                req->setParameter(param.first, param.second);
            }
            for (const auto &header: headers) {
                req->addHeader(header.first, header.second);
            }
            auto [result, responsePtr] = useSsl ?
                                         httpsClient->sendRequest(req, 10) :
                                         httpClient->sendRequest(req, 10);
            LOG_DEBUG << result;
            if (result != ReqResult::Ok) {
                throw ResponseException(
                        I18nHelper<T>::i18n("networkError"),
                        ResultCode::NetworkError,
                        drogon::k503ServiceUnavailable
                );
            }
            JsonHelper response{responsePtr};
            LOG_DEBUG << response.stringify();
            if (responsePtr->statusCode() != k200OK) {
                throw ResponseException(
                        response["message"].isString() ? response["message"].asString() : I18nHelper<T>::i18n(
                                "networkError"),
                        enum_cast<ResultCode>(response["code"].asUInt()).value_or(ResultCode::Unknown),
                        responsePtr->statusCode()
                );
            }
            try {
                for (const auto &constraint: constraints) {
                    response.require(constraint.first, constraint.second);
                }
            } catch (const exception &e) {
                throw ResponseException(
                        I18nHelper<T>::i18n("networkError"),
                        ResultCode::NetworkError,
                        drogon::k503ServiceUnavailable
                );
            }
            return response;
        }

        ~HttpRequestHandler() override = default;

    protected:
        drogon::HttpClientPtr httpClient, httpsClient;

        void setClient(const std::string &host) {
            httpClient = drogon::HttpClient::newHttpClient(host);
            httpsClient = drogon::HttpClient::newHttpClient(host, nullptr, false, true);
        }
    };
}