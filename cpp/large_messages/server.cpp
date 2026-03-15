/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Large Messages Server -- demonstrates TP segmentation.
 * Configuration is loaded from config/large_messages.yaml.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <vector>
#include <string>

#include "config.h"
#include <rpc/rpc_server.h>
#include <rpc/rpc_types.h>
#include <tp/tp_manager.h>

using namespace someip;
using namespace someip::rpc;
using namespace someip::tp;

static std::atomic<bool> running{true};
static void signal_handler(int) { running = false; }

static std::vector<uint8_t> generate_test_data(size_t size) {
    std::vector<uint8_t> data(size);
    for (size_t i = 0; i < size; ++i)
        data[i] = static_cast<uint8_t>(i % 256);
    return data;
}

static bool verify_test_data(const std::vector<uint8_t>& data) {
    for (size_t i = 0; i < data.size(); ++i)
        if (data[i] != static_cast<uint8_t>(i % 256)) return false;
    return true;
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::string config_path;
    if (argc > 2 && std::string(argv[1]) == "--config") config_path = argv[2];

    examples::Config cfg("large_messages", config_path);

    auto service_id = cfg.get_uint16("service", "service_id", 0x5000);
    auto send_id = cfg.get_uint16("service.methods", "send_large_data", 0x0001);
    auto recv_id = cfg.get_uint16("service.methods", "receive_large_data", 0x0002);
    auto echo_id = cfg.get_uint16("service.methods", "echo_large_data", 0x0003);

    RpcServer server(service_id);
    TpManager tp_manager;

    server.register_method(send_id, [](uint16_t, uint16_t,
            const std::vector<uint8_t>& in, std::vector<uint8_t>& out) -> RpcResult {
        if (in.size() < 4) return RpcResult::INVALID_PARAMETERS;
        size_t size = (in[0]<<24)|(in[1]<<16)|(in[2]<<8)|in[3];
        std::cout << "SEND_LARGE_DATA: Generating " << size << " bytes" << std::endl;
        out = generate_test_data(size);
        return RpcResult::SUCCESS;
    });

    server.register_method(recv_id, [](uint16_t, uint16_t,
            const std::vector<uint8_t>& in, std::vector<uint8_t>& out) -> RpcResult {
        bool ok = verify_test_data(in);
        std::string status = ok ? "valid" : "CORRUPTED";
        std::cout << "RECEIVE_LARGE_DATA: " << in.size() << " bytes -- " << status << std::endl;
        std::string resp = "Received " + std::to_string(in.size()) + " bytes, integrity: " + status;
        out.assign(resp.begin(), resp.end());
        return RpcResult::SUCCESS;
    });

    server.register_method(echo_id, [](uint16_t, uint16_t,
            const std::vector<uint8_t>& in, std::vector<uint8_t>& out) -> RpcResult {
        std::cout << "ECHO_LARGE_DATA: Echoing " << in.size() << " bytes" << std::endl;
        out = in;
        return RpcResult::SUCCESS;
    });

    tp_manager.initialize();
    if (!server.initialize()) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }

    std::cout << "=== SOME/IP Large Messages Server (C++) ===" << std::endl;
    std::cout << "Service 0x" << std::hex << service_id << std::dec << std::endl;
    std::cout << "Press Ctrl+C to exit\n" << std::endl;

    while (running) std::this_thread::sleep_for(std::chrono::milliseconds(100));

    tp_manager.shutdown();
    server.shutdown();
    std::cout << "Server stopped." << std::endl;
    return 0;
}
