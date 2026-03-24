/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Complex Types Server -- demonstrates serialization with structs.
 * Configuration is loaded from config/complex_types.yaml.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <string>
#include <vector>

#include "config.h"
#include <rpc/rpc_server.h>
#include <rpc/rpc_types.h>
#include <serialization/serializer.h>

using namespace someip;
using namespace someip::rpc;
using namespace someip::serialization;

static std::atomic<bool> running{true};
static void signal_handler(int) { running = false; }

struct SensorReading {
    uint8_t sensor_id;
    float value;
    std::string unit;
    uint32_t timestamp;
};

class ComplexTypesServer {
public:
    explicit ComplexTypesServer(uint16_t service_id) : server_(service_id) {}

    bool initialize(uint16_t pvd_id, uint16_t gsa_id, uint16_t ecs_id) {
        server_.register_method(pvd_id, [this](uint16_t, uint16_t,
                const std::vector<uint8_t>& in, std::vector<uint8_t>& out) -> RpcResult {
            Deserializer des(in);
            auto vid = des.deserialize_uint32();
            auto model = des.deserialize_string();
            if (vid.is_error() || model.is_error()) return RpcResult::INVALID_PARAMETERS;

            std::cout << "Processing vehicle: " << model.get_value()
                      << " (ID: " << vid.get_value() << ")" << std::endl;

            std::string resp = "Processed vehicle data for " + model.get_value() +
                               " (ID: " + std::to_string(vid.get_value()) + ")";
            Serializer ser;
            ser.serialize_string(resp);
            out = ser.get_buffer();
            return RpcResult::SUCCESS;
        });

        server_.register_method(gsa_id, [](uint16_t, uint16_t,
                const std::vector<uint8_t>&, std::vector<uint8_t>& out) -> RpcResult {
            std::vector<SensorReading> sensors = {
                {1, 23.5f, "C", 1000000},
                {2, 65.2f, "%", 1000001},
                {3, 12.8f, "V", 1000002},
                {4, 1013.25f, "hPa", 1000003},
            };
            std::cout << "Returning sensor array with " << sensors.size() << " readings" << std::endl;

            Serializer ser;
            ser.serialize_uint32(static_cast<uint32_t>(sensors.size()));
            for (auto& s : sensors) {
                ser.serialize_uint8(s.sensor_id);
                ser.serialize_float(s.value);
                ser.serialize_string(s.unit);
                ser.serialize_uint32(s.timestamp);
            }
            out = ser.get_buffer();
            return RpcResult::SUCCESS;
        });

        server_.register_method(ecs_id, [](uint16_t, uint16_t,
                const std::vector<uint8_t>& in, std::vector<uint8_t>& out) -> RpcResult {
            Deserializer des(in);
            auto sid = des.deserialize_uint8();
            auto val = des.deserialize_float();
            auto unit = des.deserialize_string();
            auto ts = des.deserialize_uint32();
            if (sid.is_error() || val.is_error() || unit.is_error() || ts.is_error())
                return RpcResult::INVALID_PARAMETERS;

            std::cout << "Echoing sensor: ID=" << (int)sid.get_value()
                      << ", Value=" << val.get_value() << unit.get_value() << std::endl;

            Serializer ser;
            ser.serialize_uint8(sid.get_value());
            ser.serialize_float(val.get_value());
            ser.serialize_string(unit.get_value());
            ser.serialize_uint32(ts.get_value());
            out = ser.get_buffer();
            return RpcResult::SUCCESS;
        });

        return server_.initialize();
    }

    void run() {
        while (running) std::this_thread::sleep_for(std::chrono::milliseconds(100));
        server_.shutdown();
    }

private:
    RpcServer server_;
};

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::string config_path;
    if (argc > 2 && std::string(argv[1]) == "--config") config_path = argv[2];

    examples::Config cfg("complex_types", config_path);

    auto service_id = cfg.get_uint16("service", "service_id", 0x4000);
    auto pvd_id = cfg.get_uint16("service.methods", "process_vehicle_data", 0x0001);
    auto gsa_id = cfg.get_uint16("service.methods", "get_sensor_array", 0x0002);
    auto ecs_id = cfg.get_uint16("service.methods", "echo_complex_struct", 0x0003);

    std::cout << "=== SOME/IP Complex Types Server (C++) ===" << std::endl;
    std::cout << "Service 0x" << std::hex << service_id << std::dec << std::endl;
    std::cout << "Press Ctrl+C to exit\n" << std::endl;

    ComplexTypesServer server(service_id);
    if (!server.initialize(pvd_id, gsa_id, ecs_id)) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }

    server.run();
    std::cout << "Server stopped." << std::endl;
    return 0;
}
