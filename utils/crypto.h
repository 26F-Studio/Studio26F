//
// Created by Parti on 2020/12/7.
//

#pragma once

#include <string>

namespace studio26f::utils::crypto {
    namespace panama {
        [[maybe_unused]] std::string generateKey();

        [[maybe_unused]] std::string generateIv();

        [[maybe_unused]] std::string encrypt(const std::string &source, std::string key, std::string iv);

        [[maybe_unused]] std::string decrypt(std::string source, std::string key, std::string iv);
    }

    std::string blake2B(const std::string &source, unsigned int divider = 2);

    std::string keccak(const std::string &source, unsigned int divider = 2);
}

