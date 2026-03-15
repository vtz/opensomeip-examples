/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Calculator RPC Client -- calls add, multiply, get_stats.
 * Configuration is loaded from config/method_calls.yaml.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>

#include "config.h"
#include <rpc/rpc_client.h>
#include <rpc/rpc_types.h>

using namespace someip;
using namespace someip::rpc;

class CalculatorClient {
public:
    explicit CalculatorClient(uint16_t client_id) : client_(client_id) {}

    bool initialize() { return client_.initialize(); }
    void shutdown() { client_.shutdown(); }

    void test_add(uint16_t service_id, uint16_t method_id, int32_t a, int32_t b) {
        std::cout << "\n--- Testing ADD(" << a << ", " << b << ") ---" << std::endl;
        std::vector<uint8_t> params(8);
        params[0]=(a>>24)&0xFF; params[1]=(a>>16)&0xFF; params[2]=(a>>8)&0xFF; params[3]=a&0xFF;
        params[4]=(b>>24)&0xFF; params[5]=(b>>16)&0xFF; params[6]=(b>>8)&0xFF; params[7]=b&0xFF;

        auto result = client_.call_method_sync(service_id, method_id, params);
        if (result.result != RpcResult::SUCCESS || result.return_values.size() < 4) {
            std::cout << "RPC call failed" << std::endl;
            return;
        }
        int32_t s = (result.return_values[0]<<24)|(result.return_values[1]<<16)|
                    (result.return_values[2]<<8)|result.return_values[3];
        std::cout << "Result: " << a << " + " << b << " = " << s << std::endl;
    }

    void test_multiply(uint16_t service_id, uint16_t method_id, int32_t a, int32_t b) {
        std::cout << "\n--- Testing MULTIPLY(" << a << ", " << b << ") ---" << std::endl;
        std::vector<uint8_t> params(8);
        params[0]=(a>>24)&0xFF; params[1]=(a>>16)&0xFF; params[2]=(a>>8)&0xFF; params[3]=a&0xFF;
        params[4]=(b>>24)&0xFF; params[5]=(b>>16)&0xFF; params[6]=(b>>8)&0xFF; params[7]=b&0xFF;

        auto result = client_.call_method_sync(service_id, method_id, params);
        if (result.result != RpcResult::SUCCESS || result.return_values.size() < 4) {
            std::cout << "RPC call failed" << std::endl;
            return;
        }
        int32_t p = (result.return_values[0]<<24)|(result.return_values[1]<<16)|
                    (result.return_values[2]<<8)|result.return_values[3];
        std::cout << "Result: " << a << " * " << b << " = " << p << std::endl;
    }

    void test_get_stats(uint16_t service_id, uint16_t method_id) {
        std::cout << "\n--- Testing GET_STATS() ---" << std::endl;
        auto result = client_.call_method_sync(service_id, method_id, {});
        if (result.result != RpcResult::SUCCESS || result.return_values.size() < 4) {
            std::cout << "RPC call failed" << std::endl;
            return;
        }
        uint32_t c = (result.return_values[0]<<24)|(result.return_values[1]<<16)|
                     (result.return_values[2]<<8)|result.return_values[3];
        std::cout << "Server statistics: " << c << " total method calls processed" << std::endl;
    }

private:
    RpcClient client_;
};

int main(int argc, char* argv[]) {
    std::string config_path;
    if (argc > 2 && std::string(argv[1]) == "--config") config_path = argv[2];

    examples::Config cfg("method_calls", config_path);

    auto service_id = cfg.get_uint16("service", "service_id", 0x2000);
    auto add_id     = cfg.get_uint16("service.methods", "add", 0x0001);
    auto mul_id     = cfg.get_uint16("service.methods", "multiply", 0x0002);
    auto stats_id   = cfg.get_uint16("service.methods", "get_stats", 0x0003);
    auto client_id  = cfg.get_uint16("network.client", "client_id", 0xABCD);

    std::cout << "=== SOME/IP Method Calls Client (C++) ===" << std::endl;
    std::cout << "Calculator Client (ID: 0x" << std::hex << client_id << ")" << std::dec << std::endl;

    CalculatorClient client(client_id);
    if (!client.initialize()) {
        std::cerr << "Failed to initialize client" << std::endl;
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "\n=== Running Calculator Operations ===" << std::endl;
    client.test_add(service_id, add_id, 10, 5);
    client.test_add(service_id, add_id, -3, 7);
    client.test_add(service_id, add_id, 1000, 2000);
    client.test_multiply(service_id, mul_id, 6, 7);
    client.test_multiply(service_id, mul_id, -4, 5);
    client.test_multiply(service_id, mul_id, 25, 4);
    client.test_get_stats(service_id, stats_id);
    std::cout << "\n=== All Operations Completed ===" << std::endl;

    client.shutdown();
    std::cout << "\nClient finished." << std::endl;
    return 0;
}
