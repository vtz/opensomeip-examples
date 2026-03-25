/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * E2E Protection for Zephyr -- CRC / counter / corruption detection demo.
 * Mirrors the host C++ e2e_protection/basic_e2e.cpp example.
 */

#include <cstdio>
#include <vector>

#include "e2e/e2e_protection.h"
#include "e2e/e2e_config.h"
#include "e2e/e2e_profiles/standard_profile.h"
#include "someip/message.h"
#include "common/result.h"

using namespace someip;
using namespace someip::e2e;

static int tests_passed = 0;
static int tests_failed = 0;

#define CHECK(cond, name)                                   \
    do {                                                    \
        if (cond) {                                         \
            printf("  [PASS] %s\n", name);                  \
            tests_passed++;                                 \
        } else {                                            \
            printf("  [FAIL] %s\n", name);                  \
            tests_failed++;                                 \
        }                                                   \
    } while (0)

int main() {
    printf("=== SOME/IP E2E Protection (Zephyr) ===\n");

    initialize_basic_profile();

    Message msg(MessageId(0x1234, 0x5678), RequestId(0x9ABC, 0xDEF0));
    msg.set_payload({0x01, 0x02, 0x03, 0x04, 0x05});

    E2EConfig e2e_cfg(0x1234);
    e2e_cfg.enable_crc = true;
    e2e_cfg.enable_counter = true;
    e2e_cfg.enable_freshness = true;
    e2e_cfg.crc_type = 1;

    E2EProtection protection;

    printf("\n--- Protect & Validate ---\n");
    Result result = protection.protect(msg, e2e_cfg);
    CHECK(result == Result::SUCCESS, "protect_message");
    CHECK(msg.has_e2e_header(), "e2e_header_present");

    std::vector<uint8_t> serialized = msg.serialize();
    printf("  Serialized: %zu bytes\n", serialized.size());
    CHECK(serialized.size() >= 16, "serialized_min_size");

    Message received_msg;
    bool deser_ok = received_msg.deserialize(serialized, true);
    CHECK(deser_ok, "deserialize");

    result = protection.validate(received_msg, e2e_cfg);
    CHECK(result == Result::SUCCESS, "validate_message");

    if (result == Result::SUCCESS) {
        auto header_opt = received_msg.get_e2e_header();
        if (header_opt.has_value()) {
            const auto& header = header_opt.value();
            printf("  CRC: 0x%X, Counter: %u, Data ID: 0x%X\n",
                   header.crc, header.counter, header.data_id);
        }
    }

    printf("\n--- Corruption Detection ---\n");
    received_msg.set_payload({0xFF, 0xFF});
    result = protection.validate(received_msg, e2e_cfg);
    CHECK(result != Result::SUCCESS, "corruption_detected");

    printf("\n=== Results: %d passed, %d failed ===\n",
           tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
