//
// Created by particleg on 2021/9/27.
//

#include <filters/CheckPermission.h>
#include <magic_enum.hpp>
#include <models/Player.h>
#include <structures/Exceptions.h>
#include <types/Permission.h>
#include <types/ResultCode.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace studio26f::filters;
using namespace studio26f::structures;
using namespace studio26f::types;

using Player = drogon_model::studio26f::Player;

void CheckPermission::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    const auto playerId = req->attributes()->get<int64_t>("playerId");
    try {
        try {
            if (enum_cast<Permission>(
                    orm::Mapper<Player>(app().getDbClient())
                            .findByPrimaryKey(playerId)
                            .getValueOfPermission()
            ).value() < Permission::Admin) {
                throw ResponseException(
                        i18n("noPermission"),
                        ResultCode::NoPermission,
                        k403Forbidden
                );
            }
        } catch (const orm::UnexpectedRows &e) {
            LOG_DEBUG << "Unexpected rows: " << e.what();
            throw ResponseException(
                    i18n("playerNotFound"),
                    ResultCode::NotFound,
                    k404NotFound
            );
        }
    } catch (const ResponseException &e) {
        e.toJson().to(failedCb);
        return;
    }
    nextCb();
}


