//
// Created by particleg on 2021/9/27.
//

#include <filters/LimitVerifyEmail.h>
#include <plugins/PlayerManager.h>

using namespace drogon;
using namespace std;
using namespace studio26f::filters;
using namespace studio26f::helpers;
using namespace studio26f::plugins;
using namespace studio26f::structures;
using namespace studio26f::types;

void LimitVerifyEmail::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    auto requestJson = req->attributes()->get<JsonHelper>("requestJson");
    try {
        if (!app().getPlugin<PlayerManager>()->verifyLimit("email", requestJson["email"].asString())) {
            JsonHelper(k429TooManyRequests, ResultCode::TooFrequent)
                    .setMessage(i18n("tooFrequent"))
                    .to(failedCb);
            return;
        }
    } catch (const exception &e) {
        LOG_ERROR << e.what();
        JsonHelper(k500InternalServerError, ResultCode::InternalError)
                .setMessage(i18n("internalError"))
                .to(failedCb);
        return;
    }
    nextCb();
}
