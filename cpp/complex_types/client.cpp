/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Complex Types Client -- demonstrates serialization with structs.
 * Configuration is loaded from config/complex_types.yaml.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>

#include "config.h"
#include <rpc/rpc_client.h>
#include <rpc/rpc_types.h>
#include <serialization/serializer.h>

using namespace someip;
using namespace someip::rpc;
using namespace someip::serialization;

class ComplexTypesClient {
public:
    explicit ComplexTypesClient(uint16_t client_id) : client_(client_id) {}

    bool initialize() { return client_.initialize(); }
    void shutdown() { client_.shutdown(); }

    void test_vehicle_data(uint16_t service_id, uint16_t method_id) {
        std::cout << "\n--- Test: Vehicle Data Processing ---" << std::endl;
        Serializer ser;
        ser.serialize_uint32(12345);
        ser.serialize_string("Tesla Model S");
        ser.serialize_float(0.85f);
        uint8_t tires[] = {32, 33, 31, 34};
        for (auto t : tires) ser.serialize_uint8(t);
        ser.serialize_bool(true);
        ser.serialize_uint16(45230);

        auto result = client_.call_method_sync(service_id, method_id, ser.get_buffer());
        if (result.result != RpcResult::SUCCESS) {
            std::cout << "RPC call failed" << std::endl;
            return;
        }
        Deserializer des(result.return_values);
        auto resp = des.deserialize_string();
        if (!resp.is_error())
            std::cout << "Server response: " << resp.get_value() << std::endl;
    }

    void test_sensor_array(uint16_t service_id, uint16_t method_id) {
        std::cout << "\n--- Test: Sensor Array Retrieval ---" << std::endl;
        auto result = client_.call_method_sync(service_id, method_id, {});
        if (result.result != RpcResult::SUCCESS) {
            std::cout << "RPC call failed" << std::endl;
            return;
        }
        Deserializer des(result.return_values);
        auto count = des.deserialize_uint32();
        if (count.is_error()) return;

        std::cout << "Received " << count.get_value() << " sensor readings:" << std::endl;
        for (uint32_t i = 0; i < count.get_value(); ++i) {
            auto sid = des.deserialize_uint8();
            auto val = des.deserialize_float();
            auto unit = des.deserialize_string();
            auto ts = des.deserialize_uint32();
            if (!sid.is_error() && !val.is_error() && !unit.is_error() && !ts.is_error()) {
                std::cout << "  Sensor " << (int)sid.get_value() << ": "
                          << val.get_value() << " " << unit.get_value()
                          << " (t=" << ts.get_value() << ")" << std::endl;
            }
        }
    }

    void test_echo_struct(uint16_t service_id, uint16_t method_id) {
        std::cout << "\n--- Test: Complex Struct Echo ---" << std::endl;
        Serializer ser;
        ser.serialize_uint8(42);
        ser.serialize_float(98.6f);
        ser.serialize_string("F");
        ser.serialize_uint32(1234567890);

        auto result = client_.call_method_sync(service_id, method_id, ser.get_buffer());
        if (result.result != RpcResult::SUCCESS) {
            std::cout << "RPC call failed" << std::endl;
            return;
        }
        Deserializer des(result.return_values);
        auto sid = des.deserialize_uint8();
        auto val = des.deserialize_float();
        auto unit = des.deserialize_string();
        auto ts = des.deserialize_uint32();
        if (!sid.is_error() && !val.is_error() && !unit.is_error() && !ts.is_error()) {
            std::cout << "Echoed: ID=" << (int)sid.get_value()
                      << ", Value=" << val.get_value() << " " << unit.get_value()
                      << ", Timestamp=" << ts.get_value() << std::endl;
        }
    }

private:
    RpcClient client_;
};

int main(int argc, char* argv[]) {
    std::string config_path;
    if (argc > 2 && std::string(argv[1]) == "--config") config_path = argv[2];

    examples::Config cfg("complex_types", config_path);

    auto service_id = cfg.get_uint16("service", "service_id", 0x4000);
    auto pvd_id = cfg.get_uint16("service.methods", "process_vehicle_data", 0x0001);
    auto gsa_id = cfg.get_uint16("service.methods", "get_sensor_array", 0x0002);
    auto ecs_id = cfg.get_uint16("service.methods", "echo_complex_struct", 0x0003);
    auto client_id = cfg.get_uint16("network.client", "client_id", 0xABCD);

    std::cout << "=== SOME/IP Complex Types Client (C++) ===" << std::endl;
    std::cout << "Client ID: 0x" << std::hex << client_id << std::dec << std::endl;

    ComplexTypesClient client(client_id);
    if (!client.initialize()) {
        std::cerr << "Failed to initialize client" << std::endl;
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "\n=== Complex Types Demonstrations ===" << std::endl;
    client.test_vehicle_data(service_id, pvd_id);
    client.test_sensor_array(service_id, gsa_id);
    client.test_echo_struct(service_id, ecs_id);
    std::cout << "\n=== All Demonstrations Completed ===" << std::endl;

    client.shutdown();
    std::cout << "\nClient finished." << std::endl;
    return 0;
}
