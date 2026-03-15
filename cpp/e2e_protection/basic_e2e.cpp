/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Basic E2E Protection -- standalone CRC / counter / corruption detection demo.
 * Configuration is loaded from config/e2e_protection.yaml.
 */

#include <iostream>
#include <vector>
#include <string>

#include "config.h"
#include "e2e/e2e_protection.h"
#include "e2e/e2e_config.h"
#include "e2e/e2e_profiles/standard_profile.h"
#include "someip/message.h"
#include "common/result.h"

using namespace someip;
using namespace someip::e2e;

int main(int argc, char* argv[]) {
    std::string config_path;
    if (argc > 2 && std::string(argv[1]) == "--config") config_path = argv[2];

    examples::Config cfg("e2e_protection", config_path);

    auto data_id = static_cast<uint16_t>(cfg.get_int("e2e", "data_id", 0x1234));

    initialize_basic_profile();

    Message msg(MessageId(0x1234, 0x5678), RequestId(0x9ABC, 0xDEF0));
    msg.set_payload({0x01, 0x02, 0x03, 0x04, 0x05});

    E2EConfig e2e_cfg(data_id);
    e2e_cfg.enable_crc = true;
    e2e_cfg.enable_counter = true;
    e2e_cfg.enable_freshness = true;
    e2e_cfg.crc_type = 1;

    E2EProtection protection;

    std::cout << "=== SOME/IP Basic E2E Protection (C++) ===" << std::endl;

    Result result = protection.protect(msg, e2e_cfg);
    if (result != Result::SUCCESS) {
        std::cerr << "Failed to protect message" << std::endl;
        return 1;
    }
    std::cout << "Message protected successfully" << std::endl;
    std::cout << "E2E header present: " << (msg.has_e2e_header() ? "yes" : "no") << std::endl;

    std::vector<uint8_t> serialized = msg.serialize();
    std::cout << "Serialized message size: " << serialized.size() << " bytes" << std::endl;

    Message received_msg;
    if (!received_msg.deserialize(serialized, true)) {
        std::cerr << "Failed to deserialize message" << std::endl;
        return 1;
    }

    result = protection.validate(received_msg, e2e_cfg);
    if (result == Result::SUCCESS) {
        std::cout << "Message validated successfully" << std::endl;
        auto header_opt = received_msg.get_e2e_header();
        if (header_opt.has_value()) {
            const auto& header = header_opt.value();
            std::cout << "E2E Header:" << std::endl;
            std::cout << "  CRC: 0x" << std::hex << header.crc << std::endl;
            std::cout << "  Counter: " << std::dec << header.counter << std::endl;
            std::cout << "  Data ID: 0x" << std::hex << header.data_id << std::endl;
        }
    } else {
        std::cerr << "Message validation failed" << std::endl;
        return 1;
    }

    // Corruption detection
    std::cout << "\nTesting corruption detection..." << std::endl;
    received_msg.set_payload({0xFF, 0xFF});
    result = protection.validate(received_msg, e2e_cfg);
    if (result != Result::SUCCESS) {
        std::cout << "Corruption correctly detected" << std::endl;
    }

    std::cout << "\n=== E2E Protection Demo Complete ===" << std::endl;
    return 0;
}
