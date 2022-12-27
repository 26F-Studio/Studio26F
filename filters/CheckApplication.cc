//
// Created by particleg on 2021/9/27.
//

#include <filters/CheckApplication.h>
#include <magic_enum.hpp>
#include <structures/Exceptions.h>
#include <types/Applications.h>
#include <types/ResultCode.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace studio26f::filters;
using namespace studio26f::helpers;
using namespace studio26f::structures;
using namespace studio26f::types;

void CheckApplication::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    const auto app = req->getHeader("x-requested-with");
    if (!app.empty()) {
        LOG_DEBUG << "Check application: " << app;
        if (!req->getPeerAddr().isIntranetIp()) {
            ResponseException(
                    i18n("noPermission"),
                    ResultCode::NoPermission,
                    k403Forbidden
            ).toJson().to(failedCb);
            return;
        }
        const auto appOptional = enum_cast<Applications>(app);
        if (!appOptional.has_value()) {
            ResponseException(
                    i18n("invalidArguments"),
                    ResultCode::InvalidArguments,
                    k400BadRequest
            ).toJson().to(failedCb);
            return;
        }
        req->attributes()->insert("app", appOptional.value());
    } else {
        req->attributes()->insert("app", Applications::Api);
    }
    nextCb();
}


