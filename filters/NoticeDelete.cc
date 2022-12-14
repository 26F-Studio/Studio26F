//
// Created by particleg on 2021/9/27.
//

#include <filters/NoticeDelete.h>

using namespace drogon;
using namespace std;
using namespace studio26f::filters;
using namespace studio26f::helpers;

void NoticeDelete::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    const auto &noticeId = req->getParameter("noticeId");
    req->attributes()->insert("noticeId", noticeId.empty() ? -1 : stoll(noticeId));
    nextCb();
}
