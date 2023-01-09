//
// Created by particleg on 2021/9/27.
//

#include <filters/NoticeRetrieve.h>
#include <helpers/JsonHelper.h>

using namespace drogon;
using namespace std;
using namespace studio26f::filters;
using namespace studio26f::helpers;
using namespace studio26f::types;

void NoticeRetrieve::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    const auto lastCount = req->getParameter("lastCount");
    const auto pageIndex = req->getParameter("pageIndex");
    const auto pageSize = req->getParameter("pageSize");
    const auto language = req->getParameter("language");

    if (language.empty()) {
        JsonHelper(k400BadRequest, ResultCode::InvalidArguments)
                .setMessage(i18n("invalidArguments"))
                .to(failedCb);
        return;
    }
    req->attributes()->insert("language", language);

    if (lastCount.empty()) {
        if (pageIndex.empty() || pageSize.empty()) {
            JsonHelper(k400BadRequest, ResultCode::InvalidArguments)
                    .setMessage(i18n("invalidArguments"))
                    .to(failedCb);
            return;
        }
        req->attributes()->insert("pageIndex", pageIndex);
        req->attributes()->insert("pageSize", pageSize);
    } else {
        req->attributes()->insert("lastCount", lastCount);
    }
    nextCb();
}


