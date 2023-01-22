//
// Created by particleg on 2021/9/27.
//

#include <filters/CheckSecure.h>
#include <helpers/JsonHelper.h>

using namespace drogon;
using namespace std;
using namespace studio26f::filters;
using namespace studio26f::helpers;
using namespace studio26f::types;

void CheckSecure::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    if (!req->isOnSecureConnection()) {
        JsonHelper(k418ImATeapot, ResultCode::Insecure)
                .setMessage(i18n("insecure"))
                .to(failedCb);
        return;
    }
    nextCb();
}
