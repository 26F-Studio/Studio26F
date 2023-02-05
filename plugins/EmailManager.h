//
// Created by g29624 on 2022/8/29.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <helpers/I18nHelper.h>

namespace studio26f::plugins {
    class EmailManager :
            public drogon::Plugin<EmailManager>,
            public helpers::I18nHelper<EmailManager> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        void smtp(
                const std::string &receiverEmail,
                const std::string &subject,
                const std::string &content
        );

    private:
        std::string _server;
        uint32_t _port;
        std::string _account, _password, _senderEmail, _senderName;
    };
}
