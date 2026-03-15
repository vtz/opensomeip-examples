/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Large Messages Client -- tests large data transfers with TP segmentation.
 * Configuration is loaded from config/large_messages.yaml.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <iomanip>

#include "config.h"
#include <rpc/rpc_client.h>
#include <rpc/rpc_types.h>
#include <tp/tp_manager.h>

using namespace someip;
using namespace someip::rpc;
using namespace someip::tp;

static std::vector<uint8_t> generate_test_data(size_t size) {
    std::vector<uint8_t> data(size);
    for (size_t i = 0; i < size; ++i)
        data[i] = static_cast<uint8_t>(i % 256);
    return data;
}

int main(int argc, char* argv[]) {
    std::string config_path;
    if (argc > 2 && std::string(argv[1]) == "--config") config_path = argv[2];

    examples::Config cfg("large_messages", config_path);

    auto service_id = cfg.get_uint16("service", "service_id", 0x5000);
    auto send_id = cfg.get_uint16("service.methods", "send_large_data", 0x0001);
    auto recv_id = cfg.get_uint16("service.methods", "receive_large_data", 0x0002);
    auto echo_id = cfg.get_uint16("service.methods", "echo_large_data", 0x0003);
    auto client_id = cfg.get_uint16("network.client", "client_id", 0xABCD);

    RpcClient client(client_id);
    TpManager tp_manager;
    tp_manager.initialize();

    if (!client.initialize()) {
        std::cerr << "Failed to initialize client" << std::endl;
        return 1;
    }

    std::cout << "=== SOME/IP Large Messages Client (C++) ===" << std::endl;
    std::cout << "Client ID: 0x" << std::hex << client_id << std::dec << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    size_t sizes[] = {2000, 10000, 50000};
    for (auto size : sizes) {
        std::cout << "\n--- Requesting " << size << " bytes ---" << std::endl;
        std::vector<uint8_t> params = {
            uint8_t((size>>24)&0xFF), uint8_t((size>>16)&0xFF),
            uint8_t((size>>8)&0xFF), uint8_t(size&0xFF)
        };
        auto result = client.call_method_sync(service_id, send_id, params);
        if (result.result == RpcResult::SUCCESS) {
            std::cout << "Received " << result.return_values.size() << " bytes" << std::endl;
            std::cout << "Size check: " << (result.return_values.size() == size ? "PASS" : "FAIL") << std::endl;
        } else {
            std::cout << "Request failed" << std::endl;
        }
    }

    // Echo test
    std::cout << "\n--- Echo round-trip: 15000 bytes ---" << std::endl;
    auto data = generate_test_data(15000);
    auto echo_result = client.call_method_sync(service_id, echo_id, data);
    if (echo_result.result == RpcResult::SUCCESS) {
        bool match = echo_result.return_values == data;
        std::cout << "Echo " << echo_result.return_values.size()
                  << " bytes -- " << (match ? "PASS" : "MISMATCH") << std::endl;
    } else {
        std::cout << "Echo failed" << std::endl;
    }

    std::cout << "\n=== All Tests Completed ===" << std::endl;

    tp_manager.shutdown();
    client.shutdown();
    std::cout << "\nClient finished." << std::endl;
    return 0;
}
