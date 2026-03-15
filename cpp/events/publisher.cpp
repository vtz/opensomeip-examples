/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sensor Event Publisher -- publishes temperature and speed events.
 * Configuration is loaded from config/events.yaml.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <random>
#include <cstring>

#include "config.h"
#include <events/event_publisher.h>
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
    auto temp_interval_ms = static_cast<int>(cfg.get_double("timing", "temperature_interval_s", 2.0) * 1000);
    auto speed_interval_ms = static_cast<int>(cfg.get_double("timing", "speed_interval_s", 1.5) * 1000);

    EventPublisher publisher(service_id, instance_id);

    EventConfig temp_cfg;
    temp_cfg.event_id = temp_event;
    temp_cfg.eventgroup_id = eventgroup;
    temp_cfg.reliability = Reliability::UNRELIABLE;
    temp_cfg.notification_type = NotificationType::PERIODIC;
    temp_cfg.cycle_time = std::chrono::milliseconds(temp_interval_ms);

    EventConfig speed_cfg;
    speed_cfg.event_id = speed_event;
    speed_cfg.eventgroup_id = eventgroup;
    speed_cfg.reliability = Reliability::UNRELIABLE;
    speed_cfg.notification_type = NotificationType::PERIODIC;
    speed_cfg.cycle_time = std::chrono::milliseconds(speed_interval_ms);

    publisher.register_event(temp_cfg);
    publisher.register_event(speed_cfg);

    if (!publisher.initialize()) {
        std::cerr << "Failed to initialize publisher" << std::endl;
        return 1;
    }

    std::cout << "=== SOME/IP Events Publisher (C++) ===" << std::endl;
    std::cout << "Service 0x" << std::hex << service_id << std::dec << std::endl;
    std::cout << "  Temperature (0x" << std::hex << temp_event << std::dec
              << ") every " << temp_interval_ms << " ms" << std::endl;
    std::cout << "  Speed       (0x" << std::hex << speed_event << std::dec
              << ") every " << speed_interval_ms << " ms" << std::endl;
    std::cout << "Press Ctrl+C to exit\n" << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> temp_dist(15.0, 35.0);
    std::uniform_real_distribution<> speed_dist(0.0, 120.0);

    auto last_temp = std::chrono::steady_clock::now();
    auto last_speed = std::chrono::steady_clock::now();

    while (running) {
        auto now = std::chrono::steady_clock::now();

        if (now - last_temp >= std::chrono::milliseconds(temp_interval_ms)) {
            float temperature = static_cast<float>(temp_dist(gen));
            uint32_t bits;
            std::memcpy(&bits, &temperature, sizeof(float));
            std::vector<uint8_t> data = {
                uint8_t((bits>>24)&0xFF), uint8_t((bits>>16)&0xFF),
                uint8_t((bits>>8)&0xFF), uint8_t(bits&0xFF)
            };
            publisher.publish_event(temp_event, data);
            std::cout << "Published Temperature: " << temperature << " C" << std::endl;
            last_temp = now;
        }

        if (now - last_speed >= std::chrono::milliseconds(speed_interval_ms)) {
            float speed = static_cast<float>(speed_dist(gen));
            uint32_t bits;
            std::memcpy(&bits, &speed, sizeof(float));
            std::vector<uint8_t> data = {
                uint8_t((bits>>24)&0xFF), uint8_t((bits>>16)&0xFF),
                uint8_t((bits>>8)&0xFF), uint8_t(bits&0xFF)
            };
            publisher.publish_event(speed_event, data);
            std::cout << "Published Speed: " << speed << " km/h" << std::endl;
            last_speed = now;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    publisher.shutdown();
    std::cout << "Publisher stopped." << std::endl;
    return 0;
}
