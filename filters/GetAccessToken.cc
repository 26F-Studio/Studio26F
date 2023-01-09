//
// Created by particleg on 2021/9/27.
//

#include <filters/GetAccessToken.h>

using namespace drogon;
using namespace std;
using namespace studio26f::filters;

void GetAccessToken::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    req->attributes()->insert("accessToken", req->getHeader("x-access-token"));
    nextCb();
}


