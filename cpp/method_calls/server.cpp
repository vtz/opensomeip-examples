/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Calculator RPC Server -- add, multiply, get_stats.
 * Configuration is loaded from config/method_calls.yaml.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <string>

#include "config.h"
#include <rpc/rpc_server.h>
#include <rpc/rpc_types.h>

using namespace someip;
using namespace someip::rpc;

static std::atomic<bool> running{true};
static void signal_handler(int) { running = false; }

class CalculatorServer {
public:
    explicit CalculatorServer(uint16_t service_id) : server_(service_id) {}

    bool initialize(uint16_t add_id, uint16_t mul_id, uint16_t stats_id) {
        server_.register_method(add_id, [this](uint16_t, uint16_t,
                const std::vector<uint8_t>& in, std::vector<uint8_t>& out) -> RpcResult {
            if (in.size() < 8) return RpcResult::INVALID_PARAMETERS;
            int32_t a = (in[0]<<24)|(in[1]<<16)|(in[2]<<8)|in[3];
            int32_t b = (in[4]<<24)|(in[5]<<16)|(in[6]<<8)|in[7];
            int32_t r = a + b;
            total_calls_++;
            std::cout << "ADD: " << a << " + " << b << " = " << r << std::endl;
            out = {uint8_t((r>>24)&0xFF), uint8_t((r>>16)&0xFF),
                   uint8_t((r>>8)&0xFF), uint8_t(r&0xFF)};
            return RpcResult::SUCCESS;
        });

        server_.register_method(mul_id, [this](uint16_t, uint16_t,
                const std::vector<uint8_t>& in, std::vector<uint8_t>& out) -> RpcResult {
            if (in.size() < 8) return RpcResult::INVALID_PARAMETERS;
            int32_t a = (in[0]<<24)|(in[1]<<16)|(in[2]<<8)|in[3];
            int32_t b = (in[4]<<24)|(in[5]<<16)|(in[6]<<8)|in[7];
            int32_t r = a * b;
            total_calls_++;
            std::cout << "MULTIPLY: " << a << " * " << b << " = " << r << std::endl;
            out = {uint8_t((r>>24)&0xFF), uint8_t((r>>16)&0xFF),
                   uint8_t((r>>8)&0xFF), uint8_t(r&0xFF)};
            return RpcResult::SUCCESS;
        });

        server_.register_method(stats_id, [this](uint16_t, uint16_t,
                const std::vector<uint8_t>&, std::vector<uint8_t>& out) -> RpcResult {
            uint32_t c = total_calls_.load();
            std::cout << "GET_STATS: " << c << " total calls processed" << std::endl;
            out = {uint8_t((c>>24)&0xFF), uint8_t((c>>16)&0xFF),
                   uint8_t((c>>8)&0xFF), uint8_t(c&0xFF)};
            return RpcResult::SUCCESS;
        });

        return server_.initialize();
    }

    void run() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        server_.shutdown();
    }

private:
    RpcServer server_;
    std::atomic<uint32_t> total_calls_{0};
};

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::string config_path;
    if (argc > 2 && std::string(argv[1]) == "--config") config_path = argv[2];

    examples::Config cfg("method_calls", config_path);

    auto service_id = cfg.get_uint16("service", "service_id", 0x2000);
    auto add_id     = cfg.get_uint16("service.methods", "add", 0x0001);
    auto mul_id     = cfg.get_uint16("service.methods", "multiply", 0x0002);
    auto stats_id   = cfg.get_uint16("service.methods", "get_stats", 0x0003);
    auto port       = cfg.get_uint16("network.server", "port", 30491);

    std::cout << "=== SOME/IP Method Calls Server (C++) ===" << std::endl;
    std::cout << "Calculator service 0x" << std::hex << service_id
              << std::dec << " on port " << port << std::endl;
    std::cout << "Press Ctrl+C to exit\n" << std::endl;

    CalculatorServer server(service_id);
    if (!server.initialize(add_id, mul_id, stats_id)) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }

    server.run();
    std::cout << "Server stopped." << std::endl;
    return 0;
}
