//
// Created by particleg on 2021/9/27.
//

#include <filters/AuthOauth.h>
#include <helpers/JsonHelper.h>

using namespace drogon;
using namespace std;
using namespace studio26f::filters;
using namespace studio26f::helpers;
using namespace studio26f::types;

void AuthOauth::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    handleExceptions([&]() {
        auto request = JsonHelper(req);
        request.require("product", JsonValue::String);
        request.require("recaptcha", JsonValue::String);
        req->attributes()->insert("requestJson", request);
        nextCb();
    }, failedCb);
}
