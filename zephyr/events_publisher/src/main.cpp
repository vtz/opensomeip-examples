/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sensor Event Publisher for Zephyr -- publishes temperature and speed events.
 * Mirrors the host C++ events/publisher.cpp example.
 */

#include <cstdio>
#include <cstring>
#include <vector>
#include <chrono>

#include "events/event_publisher.h"
#include "events/event_types.h"
#include "platform/thread.h"

using namespace someip;
using namespace someip::events;

static constexpr uint16_t SERVICE_ID    = 0x3000;
static constexpr uint16_t INSTANCE_ID   = 0x0001;
static constexpr uint16_t TEMP_EVENT    = 0x8001;
static constexpr uint16_t SPEED_EVENT   = 0x8002;
static constexpr uint16_t EVENTGROUP    = 0x0001;
static constexpr int TEMP_INTERVAL_MS   = 2000;
static constexpr int SPEED_INTERVAL_MS  = 1500;

static uint32_t xorshift_state = 0xDEADBEEF;

static uint32_t xorshift32() {
    xorshift_state ^= xorshift_state << 13;
    xorshift_state ^= xorshift_state >> 17;
    xorshift_state ^= xorshift_state << 5;
    return xorshift_state;
}

static float random_range(float lo, float hi) {
    return lo + (hi - lo) * (static_cast<float>(xorshift32() & 0xFFFF) / 65535.0f);
}

int main() {
    printf("=== SOME/IP Events Publisher (Zephyr) ===\n");
    printf("Service 0x%04X\n", SERVICE_ID);
    printf("  Temperature (0x%04X) every %d ms\n", TEMP_EVENT, TEMP_INTERVAL_MS);
    printf("  Speed       (0x%04X) every %d ms\n", SPEED_EVENT, SPEED_INTERVAL_MS);

    EventPublisher publisher(SERVICE_ID, INSTANCE_ID);

    EventConfig temp_cfg;
    temp_cfg.event_id = TEMP_EVENT;
    temp_cfg.eventgroup_id = EVENTGROUP;
    temp_cfg.reliability = Reliability::UNRELIABLE;
    temp_cfg.notification_type = NotificationType::PERIODIC;
    temp_cfg.cycle_time = std::chrono::milliseconds(TEMP_INTERVAL_MS);

    EventConfig speed_cfg;
    speed_cfg.event_id = SPEED_EVENT;
    speed_cfg.eventgroup_id = EVENTGROUP;
    speed_cfg.reliability = Reliability::UNRELIABLE;
    speed_cfg.notification_type = NotificationType::PERIODIC;
    speed_cfg.cycle_time = std::chrono::milliseconds(SPEED_INTERVAL_MS);

    publisher.register_event(temp_cfg);
    publisher.register_event(speed_cfg);

    if (!publisher.initialize()) {
        printf("[publisher] Failed to initialize\n");
        return 1;
    }

    printf("[publisher] Publishing events...\n");

    auto last_temp  = std::chrono::steady_clock::now();
    auto last_speed = std::chrono::steady_clock::now();

    while (true) {
        auto now = std::chrono::steady_clock::now();

        if (now - last_temp >= std::chrono::milliseconds(TEMP_INTERVAL_MS)) {
            float temperature = random_range(15.0f, 35.0f);
            uint32_t bits;
            std::memcpy(&bits, &temperature, sizeof(float));
            std::vector<uint8_t> data = {
                uint8_t((bits>>24)&0xFF), uint8_t((bits>>16)&0xFF),
                uint8_t((bits>>8)&0xFF),  uint8_t(bits&0xFF)
            };
            publisher.publish_event(TEMP_EVENT, data);
            printf("[publisher] Temperature: %.1f C\n", static_cast<double>(temperature));
            last_temp = now;
        }

        if (now - last_speed >= std::chrono::milliseconds(SPEED_INTERVAL_MS)) {
            float speed = random_range(0.0f, 120.0f);
            uint32_t bits;
            std::memcpy(&bits, &speed, sizeof(float));
            std::vector<uint8_t> data = {
                uint8_t((bits>>24)&0xFF), uint8_t((bits>>16)&0xFF),
                uint8_t((bits>>8)&0xFF),  uint8_t(bits&0xFF)
            };
            publisher.publish_event(SPEED_EVENT, data);
            printf("[publisher] Speed: %.1f km/h\n", static_cast<double>(speed));
            last_speed = now;
        }

        platform::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
