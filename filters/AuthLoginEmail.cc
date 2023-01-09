//
// Created by particleg on 2021/9/27.
//

#include <filters/AuthLoginEmail.h>
#include <helpers/JsonHelper.h>
#include <structures/Exceptions.h>

using namespace drogon;
using namespace std;
using namespace studio26f::filters;
using namespace studio26f::helpers;
using namespace studio26f::structures;
using namespace studio26f::types;

void AuthLoginEmail::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    handleExceptions([&]() {
        auto request = JsonHelper(req);
        request.require("email", JsonValue::String);
        if (!(
                request.check("code", JsonValue::String) ||
                request.check("password", JsonValue::String)
        )) {
            throw json_exception::WrongType(JsonValue::String);
        }
        req->attributes()->insert("requestJson", request);
        nextCb();
    }, failedCb);
}
