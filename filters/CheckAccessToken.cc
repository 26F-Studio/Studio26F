//
// Created by particleg on 2021/9/27.
//

#include <filters/CheckAccessToken.h>
#include <helpers/ResponseJson.h>
#include <plugins/PlayerManager.h>
#include <structures/Exceptions.h>
#include <types/ResultCode.h>

using namespace drogon;
using namespace std;
using namespace studio26f::filters;
using namespace studio26f::helpers;
using namespace studio26f::plugins;
using namespace studio26f::structures;
using namespace studio26f::types;

void CheckAccessToken::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    const auto accessToken = req->getHeader("x-access-token");
    if (accessToken.empty()) {
        ResponseJson(k400BadRequest, ResultCode::InvalidArguments)
                .setMessage(i18n("invalidArguments"))
                .to(failedCb);
        return;
    }
    try {
        req->attributes()->insert(
                "playerId",
                app().getPlugin<PlayerManager>()->getPlayerIdByAccessToken(accessToken)
        );
    } catch (const ResponseException &e) {
        e.toJson().to(failedCb);
        return;
    }
    nextCb();
}


