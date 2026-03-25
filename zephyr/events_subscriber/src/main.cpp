/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sensor Event Subscriber for Zephyr -- receives temperature and speed events.
 * Mirrors the host C++ events/subscriber.cpp example.
 */

#include <cstdio>
#include <cstring>
#include <atomic>

#include "events/event_subscriber.h"
#include "events/event_types.h"
#include "platform/thread.h"

using namespace someip;
using namespace someip::events;

static constexpr uint16_t SERVICE_ID  = 0x3000;
static constexpr uint16_t INSTANCE_ID = 0x0001;
static constexpr uint16_t TEMP_EVENT  = 0x8001;
static constexpr uint16_t SPEED_EVENT = 0x8002;
static constexpr uint16_t EVENTGROUP  = 0x0001;

static std::atomic<uint32_t> events_received{0};

int main() {
    printf("=== SOME/IP Events Subscriber (Zephyr) ===\n");
    printf("Service 0x%04X\n", SERVICE_ID);
    printf("  Subscribed to Temperature (0x%04X)\n", TEMP_EVENT);
    printf("  Subscribed to Speed       (0x%04X)\n", SPEED_EVENT);

    EventSubscriber subscriber(0x0001);

    if (!subscriber.initialize()) {
        printf("[subscriber] Failed to initialize\n");
        return 1;
    }

    subscriber.subscribe_eventgroup(SERVICE_ID, INSTANCE_ID, EVENTGROUP,
        [](const EventNotification& n) {
            if (n.event_data.size() < 4) return;
            uint32_t bits = (n.event_data[0]<<24)|(n.event_data[1]<<16)|
                            (n.event_data[2]<<8)|n.event_data[3];
            float val;
            std::memcpy(&val, &bits, sizeof(float));

            if (n.event_id == TEMP_EVENT) {
                printf("[subscriber] Temperature: %.1f C\n", static_cast<double>(val));
            } else if (n.event_id == SPEED_EVENT) {
                printf("[subscriber] Speed: %.1f km/h\n", static_cast<double>(val));
            }
            events_received.fetch_add(1);
        });

    printf("[subscriber] Waiting for events...\n");

    while (true) {
        platform::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    subscriber.shutdown();
    return 0;
}
