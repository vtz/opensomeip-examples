/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Hello World Client -- sends a greeting and prints the server response.
 * Configuration is loaded from config/hello_world.yaml.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <string>

#include "config.h"
#include <transport/udp_transport.h>
#include <transport/endpoint.h>
#include <someip/message.h>

using namespace someip;
using namespace someip::transport;

static std::mutex response_mutex;
static std::condition_variable response_cv;
static bool response_received = false;
static std::string server_response;

class HelloClient : public ITransportListener {
public:
    HelloClient(uint16_t service_id, uint16_t method_id)
        : transport_(std::make_shared<UdpTransport>(Endpoint("0.0.0.0", 0)))
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

    void send_hello(const std::string& message, const std::string& host, uint16_t port) {
        Message request(MessageId(service_id_, method_id_),
                        RequestId(0x1234, 0x5678),
                        MessageType::REQUEST, ReturnCode::E_OK);
        request.set_payload(std::vector<uint8_t>(message.begin(), message.end()));

        Endpoint server_ep(host, port);
        std::cout << "Sending message: '" << message << "' to "
                  << server_ep.to_string() << std::endl;
        if (transport_->send_message(request, server_ep) != Result::SUCCESS) {
            std::cerr << "Failed to send message" << std::endl;
        }
    }

    void wait_for_response(int timeout_ms = 5000) {
        std::unique_lock<std::mutex> lock(response_mutex);
        if (response_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                                 [] { return response_received; })) {
            std::cout << "Server responded: '" << server_response << "'" << std::endl;
        } else {
            std::cout << "Timeout waiting for server response" << std::endl;
        }
    }

    void on_message_received(MessagePtr message, const Endpoint&) override {
        if (message->get_service_id() == service_id_ &&
            message->get_method_id() == method_id_ &&
            message->is_response()) {
            std::string text(message->get_payload().begin(),
                             message->get_payload().end());
            {
                std::lock_guard<std::mutex> lock(response_mutex);
                server_response = text;
                response_received = true;
            }
            response_cv.notify_one();
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
    std::string config_path;
    if (argc > 2 && std::string(argv[1]) == "--config") {
        config_path = argv[2];
    }

    examples::Config cfg("hello_world", config_path);

    auto server_host = cfg.get_string("network.client", "server_host", "127.0.0.1");
    auto server_port = cfg.get_uint16("network.client", "server_port", 30490);
    auto service_id = cfg.get_uint16("service", "service_id", 0x1000);
    auto method_id = cfg.get_uint16("service.methods", "say_hello", 0x0001);

    std::cout << "=== SOME/IP Hello World Client (C++) ===" << std::endl << std::endl;

    HelloClient client(service_id, method_id);
    if (!client.start()) return 1;

    client.send_hello("Hello from C++ Client!", server_host, server_port);
    client.wait_for_response();

    client.stop();
    std::cout << "Client finished." << std::endl;
    return 0;
}
