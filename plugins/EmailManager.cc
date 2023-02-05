//
// Created by g29624 on 2022/8/29.
//

#include <drogon/HttpAppFramework.h>
#include <magic_enum.hpp>
#include <mailio/smtp.hpp>
#include <plugins/EmailManager.h>
#include <structures/Exceptions.h>

using namespace drogon;
using namespace mailio;
using namespace magic_enum;
using namespace std;
using namespace studio26f::helpers;
using namespace studio26f::plugins;
using namespace studio26f::structures;
using namespace studio26f::types;

void EmailManager::initAndStart(const Json::Value &config) {
    if (!(
            config["server"].isString() &&
            config["port"].isUInt() &&
            config["account"].isString() &&
            config["password"].isString() &&
            config["senderEmail"].isString() &&
            config["senderName"].isString()
    )) {
        LOG_ERROR << R"(Invalid config)";
        abort();
    }
    _server = config["server"].asString();
    _port = config["port"].asUInt();
    _account = config["account"].asString();
    _password = config["password"].asString();
    _senderEmail = config["senderEmail"].asString();
    _senderName = config["senderName"].asString();

    LOG_INFO << "EmailManager loaded.";
}

void EmailManager::shutdown() {
    LOG_INFO << "EmailManager shutdown.";
}

void EmailManager::smtp(
        const string &receiverEmail,
        const string &subject,
        const string &content
) {
    try {
        message msg;
        msg.header_codec(message::header_codec_t::BASE64);
        msg.from({_senderName, _senderEmail});
        msg.add_recipient({_senderName, receiverEmail});
        msg.subject(subject);
        msg.content_transfer_encoding(mime::content_transfer_encoding_t::BINARY);
        msg.content_type(message::media_type_t::TEXT, "html", "utf-8");
        msg.content(content);
        smtps conn(_server, _port);
        conn.authenticate(_account, _password, smtps::auth_method_t::START_TLS);
        conn.submit(msg);
    } catch (smtp_error &e) {
        LOG_WARN << "SMTP Error: " << e.what();
        throw ResponseException(
                i18n("emailError"),
                e,
                ResultCode::EmailError,
                k503ServiceUnavailable
        );
    } catch (dialog_error &e) {
        LOG_WARN << "Dialog Error: " << e.what();
        throw ResponseException(
                i18n("emailError"),
                e,
                ResultCode::EmailError,
                k503ServiceUnavailable
        );
    } catch (message_error &e) {
        LOG_WARN << "Message Error: " << e.what();
        throw ResponseException(
                i18n("emailError"),
                e,
                ResultCode::EmailError,
                k503ServiceUnavailable
        );
    }
}

