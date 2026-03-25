/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Hello World Server for Zephyr -- simplest SOME/IP request/response over UDP.
 * Mirrors the host C++ hello_world/server.cpp example.
 */

#include <cstdio>
#include <string>
#include <vector>
#include <atomic>

#include "someip/message.h"
#include "someip/types.h"
#include "common/result.h"
#include "transport/endpoint.h"
#include "transport/udp_transport.h"
#include "platform/thread.h"

using namespace someip;
using namespace someip::transport;

static constexpr uint16_t SERVICE_ID  = 0x1000;
static constexpr uint16_t METHOD_ID   = 0x0001;
static constexpr uint16_t LISTEN_PORT = 30490;

class HelloServer : public ITransportListener {
public:
    HelloServer() : transport_(Endpoint("0.0.0.0", LISTEN_PORT)) {
        transport_.set_listener(this);
    }

    bool start() {
        if (transport_.start() != Result::SUCCESS) {
            printf("[server] Failed to start transport\n");
            return false;
        }
        auto ep = transport_.get_local_endpoint();
        printf("[server] Listening on %s:%d\n",
               ep.get_address().c_str(), ep.get_port());
        return true;
    }

    void stop() { transport_.stop(); }
    uint32_t messages_handled() const { return count_.load(); }

    void on_message_received(MessagePtr message, const Endpoint& sender) override {
        if (message->get_service_id() != SERVICE_ID ||
            message->get_method_id() != METHOD_ID ||
            !message->is_request()) {
            return;
        }

        std::string received_text(message->get_payload().begin(),
                                  message->get_payload().end());
        printf("[server] Client said: '%s' (from %s:%d)\n",
               received_text.c_str(),
               sender.get_address().c_str(), sender.get_port());

        std::string greeting = "Hello World! Server received: " + received_text;
        Message response(
            MessageId(SERVICE_ID, METHOD_ID),
            RequestId(message->get_client_id(), message->get_session_id()),
            MessageType::RESPONSE, ReturnCode::E_OK);
        response.set_payload(std::vector<uint8_t>(greeting.begin(), greeting.end()));

        if (transport_.send_message(response, sender) != Result::SUCCESS) {
            printf("[server] Failed to send response\n");
            return;
        }
        printf("[server] Sent greeting: '%s'\n", greeting.c_str());
        count_.fetch_add(1);
    }

    void on_connection_lost(const Endpoint&) override {}
    void on_connection_established(const Endpoint&) override {}
    void on_error(Result) override {}

private:
    UdpTransport transport_;
    std::atomic<uint32_t> count_{0};
};

int main() {
    printf("=== SOME/IP Hello World Server (Zephyr) ===\n");
    printf("Service 0x%04X, method 0x%04X on port %d\n",
           SERVICE_ID, METHOD_ID, LISTEN_PORT);

    HelloServer server;
    if (!server.start()) {
        return 1;
    }

    printf("[server] Waiting for requests...\n");

    while (true) {
        platform::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
