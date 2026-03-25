/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Calculator RPC Server for Zephyr -- add, multiply, get_stats.
 * Mirrors the host C++ method_calls/server.cpp example.
 */

#include <cstdio>
#include <vector>
#include <atomic>

#include "rpc/rpc_server.h"
#include "rpc/rpc_types.h"
#include "platform/thread.h"

using namespace someip;
using namespace someip::rpc;

static constexpr uint16_t SERVICE_ID = 0x2000;
static constexpr uint16_t ADD_ID     = 0x0001;
static constexpr uint16_t MUL_ID    = 0x0002;
static constexpr uint16_t STATS_ID  = 0x0003;

static std::atomic<uint32_t> total_calls{0};

int main() {
    printf("=== SOME/IP Method Calls Server (Zephyr) ===\n");
    printf("Calculator service 0x%04X\n", SERVICE_ID);

    RpcServer server(SERVICE_ID);

    server.register_method(ADD_ID, [](uint16_t, uint16_t,
            const std::vector<uint8_t>& in, std::vector<uint8_t>& out) -> RpcResult {
        if (in.size() < 8) return RpcResult::INVALID_PARAMETERS;
        int32_t a = (in[0]<<24)|(in[1]<<16)|(in[2]<<8)|in[3];
        int32_t b = (in[4]<<24)|(in[5]<<16)|(in[6]<<8)|in[7];
        int32_t r = a + b;
        total_calls++;
        printf("[server] ADD: %d + %d = %d\n", a, b, r);
        out = {uint8_t((r>>24)&0xFF), uint8_t((r>>16)&0xFF),
               uint8_t((r>>8)&0xFF), uint8_t(r&0xFF)};
        return RpcResult::SUCCESS;
    });

    server.register_method(MUL_ID, [](uint16_t, uint16_t,
            const std::vector<uint8_t>& in, std::vector<uint8_t>& out) -> RpcResult {
        if (in.size() < 8) return RpcResult::INVALID_PARAMETERS;
        int32_t a = (in[0]<<24)|(in[1]<<16)|(in[2]<<8)|in[3];
        int32_t b = (in[4]<<24)|(in[5]<<16)|(in[6]<<8)|in[7];
        int32_t r = a * b;
        total_calls++;
        printf("[server] MULTIPLY: %d * %d = %d\n", a, b, r);
        out = {uint8_t((r>>24)&0xFF), uint8_t((r>>16)&0xFF),
               uint8_t((r>>8)&0xFF), uint8_t(r&0xFF)};
        return RpcResult::SUCCESS;
    });

    server.register_method(STATS_ID, [](uint16_t, uint16_t,
            const std::vector<uint8_t>&, std::vector<uint8_t>& out) -> RpcResult {
        uint32_t c = total_calls.load();
        printf("[server] GET_STATS: %u total calls\n", c);
        out = {uint8_t((c>>24)&0xFF), uint8_t((c>>16)&0xFF),
               uint8_t((c>>8)&0xFF), uint8_t(c&0xFF)};
        return RpcResult::SUCCESS;
    });

    if (!server.initialize()) {
        printf("[server] Failed to initialize\n");
        return 1;
    }

    printf("[server] Waiting for RPC calls...\n");

    while (true) {
        platform::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    server.shutdown();
    return 0;
}
