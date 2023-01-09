//
// Created by particleg on 2021/9/27.
//

#include <filters/LimitIp.h>
#include <plugins/PlayerManager.h>

using namespace drogon;
using namespace std;
using namespace studio26f::filters;
using namespace studio26f::helpers;
using namespace studio26f::plugins;
using namespace studio26f::structures;
using namespace studio26f::types;

void LimitIp::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    try {
        if (!app().getPlugin<PlayerManager>()->ipLimit(req->getPeerAddr().toIp())) {
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
