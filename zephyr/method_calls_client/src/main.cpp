/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Calculator RPC Client for Zephyr -- calls add, multiply, get_stats.
 * Mirrors the host C++ method_calls/client.cpp example.
 */

#include <cstdio>
#include <vector>
#include <cstdint>

#include "rpc/rpc_client.h"
#include "rpc/rpc_types.h"
#include "platform/thread.h"

using namespace someip;
using namespace someip::rpc;

static constexpr uint16_t SERVICE_ID = 0x2000;
static constexpr uint16_t ADD_ID     = 0x0001;
static constexpr uint16_t MUL_ID    = 0x0002;
static constexpr uint16_t STATS_ID  = 0x0003;
static constexpr uint16_t CLIENT_ID = 0xABCD;

static int tests_passed = 0;
static int tests_total  = 0;

static void test_add(RpcClient& client, int32_t a, int32_t b) {
    tests_total++;
    printf("\n[client] ADD(%d, %d)\n", a, b);
    std::vector<uint8_t> params(8);
    params[0]=(a>>24)&0xFF; params[1]=(a>>16)&0xFF;
    params[2]=(a>>8)&0xFF;  params[3]=a&0xFF;
    params[4]=(b>>24)&0xFF; params[5]=(b>>16)&0xFF;
    params[6]=(b>>8)&0xFF;  params[7]=b&0xFF;

    auto result = client.call_method_sync(SERVICE_ID, ADD_ID, params);
    if (result.result != RpcResult::SUCCESS || result.return_values.size() < 4) {
        printf("[client] ADD failed\n");
        return;
    }
    int32_t s = (result.return_values[0]<<24)|(result.return_values[1]<<16)|
                (result.return_values[2]<<8)|result.return_values[3];
    printf("[client] Result: %d + %d = %d\n", a, b, s);
    if (s == a + b) tests_passed++;
}

static void test_multiply(RpcClient& client, int32_t a, int32_t b) {
    tests_total++;
    printf("\n[client] MULTIPLY(%d, %d)\n", a, b);
    std::vector<uint8_t> params(8);
    params[0]=(a>>24)&0xFF; params[1]=(a>>16)&0xFF;
    params[2]=(a>>8)&0xFF;  params[3]=a&0xFF;
    params[4]=(b>>24)&0xFF; params[5]=(b>>16)&0xFF;
    params[6]=(b>>8)&0xFF;  params[7]=b&0xFF;

    auto result = client.call_method_sync(SERVICE_ID, MUL_ID, params);
    if (result.result != RpcResult::SUCCESS || result.return_values.size() < 4) {
        printf("[client] MULTIPLY failed\n");
        return;
    }
    int32_t p = (result.return_values[0]<<24)|(result.return_values[1]<<16)|
                (result.return_values[2]<<8)|result.return_values[3];
    printf("[client] Result: %d * %d = %d\n", a, b, p);
    if (p == a * b) tests_passed++;
}

static void test_get_stats(RpcClient& client) {
    tests_total++;
    printf("\n[client] GET_STATS()\n");
    auto result = client.call_method_sync(SERVICE_ID, STATS_ID, {});
    if (result.result != RpcResult::SUCCESS || result.return_values.size() < 4) {
        printf("[client] GET_STATS failed\n");
        return;
    }
    uint32_t c = (result.return_values[0]<<24)|(result.return_values[1]<<16)|
                 (result.return_values[2]<<8)|result.return_values[3];
    printf("[client] Server stats: %u total calls\n", c);
    if (c > 0) tests_passed++;
}

int main() {
    printf("=== SOME/IP Method Calls Client (Zephyr) ===\n");

    RpcClient client(CLIENT_ID);
    if (!client.initialize()) {
        printf("[client] Failed to initialize\n");
        return 1;
    }

    platform::this_thread::sleep_for(std::chrono::seconds(1));

    printf("\n=== Running Calculator Operations ===\n");
    test_add(client, 10, 5);
    test_add(client, -3, 7);
    test_multiply(client, 6, 7);
    test_multiply(client, 25, 4);
    test_get_stats(client);

    client.shutdown();

    printf("\n=== Result: %d/%d passed ===\n", tests_passed, tests_total);
    return (tests_passed == tests_total) ? 0 : 1;
}
