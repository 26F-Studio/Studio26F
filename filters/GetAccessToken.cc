//
// Created by particleg on 2021/9/27.
//

#include <filters/GetAccessToken.h>
#include <helpers/ResponseJson.h>
#include <types/ResultCode.h>

using namespace drogon;
using namespace std;
using namespace studio26f::filters;
using namespace studio26f::helpers;
using namespace studio26f::types;

void GetAccessToken::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    req->attributes()->insert("accessToken", req->getHeader("x-access-token"));
    nextCb();
}


