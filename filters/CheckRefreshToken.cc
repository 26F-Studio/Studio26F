//
// Created by particleg on 2021/9/27.
//

#include <filters/CheckRefreshToken.h>
#include <helpers/ResponseJson.h>
#include <types/ResultCode.h>

using namespace drogon;
using namespace std;
using namespace studio26f::filters;
using namespace studio26f::helpers;
using namespace studio26f::types;

void CheckRefreshToken::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    auto refreshToken = req->getHeader("x-refresh-token");
    if (refreshToken.empty()) {
        ResponseJson(k400BadRequest, ResultCode::InvalidArguments)
                .setMessage(i18n("invalidArguments"))
                .to(failedCb);
        return;
    }
    req->attributes()->insert("refreshToken", refreshToken);
    nextCb();
}


