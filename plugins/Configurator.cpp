//
// Created by Parti on 2021/2/4.
//

#include <drogon/drogon.h>
#include <plugins/Configurator.h>

using namespace drogon;
using namespace std;
using namespace studio26f::plugins;

void Configurator::initAndStart(const Json::Value &config) {
    if (config["cors"].isBool() && config["cors"].asBool()) {
        app().registerSyncAdvice([](const HttpRequestPtr &req) -> HttpResponsePtr {
            if (req->method() == Options) {
                auto resp = HttpResponse::newHttpResponse();
                resp->addHeader("Access-Control-Allow-Origin", req->getHeader("Origin"));
                resp->addHeader("Access-Control-Allow-Headers", req->getHeader("Access-Control-Request-Headers"));
                resp->addHeader("Access-Control-Allow-Methods", req->getHeader("Access-Control-Request-Method"));
                return resp;
            }
            return nullptr;
        });
        app().registerHttpResponseCreationAdvice(
                [](const HttpResponsePtr &resp) {
                    resp->addHeader("Access-Control-Allow-Origin", "*");
                    resp->addHeader("Access-Control-Allow-Headers", "*");
                    resp->addHeader("Access-Control-Allow-Methods", "*");
                }
        );
    }
    LOG_INFO << "Configurator loaded.";
}

void Configurator::shutdown() { LOG_INFO << "Configurator shutdown."; }