/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Hello World Server -- simplest SOME/IP request/response over UDP.
 * Configuration is loaded from config/hello_world.yaml.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <string>

#include "config.h"
#include <transport/udp_transport.h>
#include <transport/endpoint.h>
#include <someip/message.h>

using namespace someip;
using namespace someip::transport;

static std::atomic<bool> running{true};

static void signal_handler(int) { running = false; }

class HelloServer : public ITransportListener {
public:
    HelloServer(const std::string& host, uint16_t port,
                uint16_t service_id, uint16_t method_id)
        : transport_(std::make_shared<UdpTransport>(Endpoint(host, port)))
        , service_id_(service_id)
        , method_id_(method_id) {
        transport_->set_listener(this);
    }

    bool start() {
        if (transport_->start() != Result::SUCCESS) {
            std::cerr << "Failed to start transport" << std::endl;
            return false;
        }
        return true;
    }

    void stop() { transport_->stop(); }

    void on_message_received(MessagePtr message, const Endpoint& sender) override {
        if (message->get_service_id() == service_id_ &&
            message->get_method_id() == method_id_ &&
            message->is_request()) {

            std::string received_text(message->get_payload().begin(),
                                      message->get_payload().end());
            std::cout << "Client said: '" << received_text
                      << "' (from " << sender.to_string() << ")" << std::endl;

            std::string greeting = "Hello World! Server received: " + received_text;
            Message response(MessageId(service_id_, method_id_),
                             RequestId(message->get_client_id(), message->get_session_id()),
                             MessageType::RESPONSE, ReturnCode::E_OK);
            response.set_payload(std::vector<uint8_t>(greeting.begin(), greeting.end()));
            transport_->send_message(response, sender);
            std::cout << "Sent greeting: '" << greeting << "'" << std::endl;
        }
    }

    void on_connection_lost(const Endpoint&) override {}
    void on_connection_established(const Endpoint&) override {}
    void on_error(Result) override {}

private:
    std::shared_ptr<UdpTransport> transport_;
    uint16_t service_id_;
    uint16_t method_id_;
};

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::string config_path;
    if (argc > 2 && std::string(argv[1]) == "--config") {
        config_path = argv[2];
    }

    examples::Config cfg("hello_world", config_path);

    auto bind_host = cfg.get_string("network.server", "host", "0.0.0.0");
    auto bind_port = cfg.get_uint16("network.server", "port", 30490);
    auto service_id = cfg.get_uint16("service", "service_id", 0x1000);
    auto method_id = cfg.get_uint16("service.methods", "say_hello", 0x0001);

    HelloServer server(bind_host, bind_port, service_id, method_id);

    std::cout << "=== SOME/IP Hello World Server (C++) ===" << std::endl;
    std::cout << "Listening on " << bind_host << ":" << bind_port << std::endl;
    std::cout << "Service 0x" << std::hex << service_id
              << ", method 0x" << method_id << std::dec << std::endl;
    std::cout << "Press Ctrl+C to exit\n" << std::endl;

    if (!server.start()) return 1;

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "\nShutting down..." << std::endl;
    server.stop();
    std::cout << "Server stopped." << std::endl;
    return 0;
}
