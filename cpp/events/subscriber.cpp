/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sensor Event Subscriber -- receives temperature and speed events.
 * Configuration is loaded from config/events.yaml.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <cstring>

#include "config.h"
#include <events/event_subscriber.h>
#include <events/event_types.h>

using namespace someip;
using namespace someip::events;

static std::atomic<bool> running{true};
static void signal_handler(int) { running = false; }

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::string config_path;
    if (argc > 2 && std::string(argv[1]) == "--config") config_path = argv[2];

    examples::Config cfg("events", config_path);

    auto service_id = cfg.get_uint16("service", "service_id", 0x3000);
    auto instance_id = cfg.get_uint16("service", "instance_id", 0x0001);
    auto temp_event = cfg.get_uint16("service.events", "temperature", 0x8001);
    auto speed_event = cfg.get_uint16("service.events", "speed", 0x8002);
    auto eventgroup = cfg.get_uint16("service.eventgroups", "sensors", 0x0001);

    EventSubscriber subscriber(0x0001);

    if (!subscriber.initialize()) {
        std::cerr << "Failed to initialize subscriber" << std::endl;
        return 1;
    }

    subscriber.subscribe_eventgroup(service_id, instance_id, eventgroup,
        [temp_event, speed_event](const EventNotification& n) {
            if (n.event_data.size() < 4) return;
            uint32_t bits = (n.event_data[0]<<24)|(n.event_data[1]<<16)|
                            (n.event_data[2]<<8)|n.event_data[3];
            float val;
            std::memcpy(&val, &bits, sizeof(float));

            if (n.event_id == temp_event) {
                std::cout << "Temperature Event: " << val << " C" << std::endl;
            } else if (n.event_id == speed_event) {
                std::cout << "Speed Event: " << val << " km/h" << std::endl;
            }
        });

    std::cout << "=== SOME/IP Events Subscriber (C++) ===" << std::endl;
    std::cout << "Service 0x" << std::hex << service_id << std::dec << std::endl;
    std::cout << "  Subscribed to Temperature (0x" << std::hex << temp_event << ")" << std::dec << std::endl;
    std::cout << "  Subscribed to Speed       (0x" << std::hex << speed_event << ")" << std::dec << std::endl;
    std::cout << "Waiting for events... Press Ctrl+C to exit\n" << std::endl;

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    subscriber.shutdown();
    std::cout << "Subscriber stopped." << std::endl;
    return 0;
}
