/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * SD Demo Server -- offers a service via Service Discovery on multicast.
 * Configuration is loaded from config/sd_demo.yaml.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <string>
#include <vector>

#include "config.h"
#include <sd/sd_server.h>
#include <sd/sd_types.h>
#include <transport/udp_transport.h>
#include <transport/endpoint.h>
#include <someip/message.h>

using namespace someip;
using namespace someip::sd;
using namespace someip::transport;

static volatile std::sig_atomic_t running = 1;
static void signal_handler(int) { running = 0; }

class RequestHandler : public ITransportListener {
public:
    RequestHandler(std::shared_ptr<UdpTransport> tp,
                   uint16_t service_id, uint16_t method_id)
        : transport_(tp), service_id_(service_id), method_id_(method_id) {}

    void on_message_received(MessagePtr msg, const Endpoint& sender) override {
        if (msg->get_service_id() != service_id_ ||
            msg->get_method_id() != method_id_ || !msg->is_request())
            return;

        std::string text(msg->get_payload().begin(), msg->get_payload().end());
        std::cout << "[service] Request from " << sender.to_string()
                  << ": '" << text << "'" << std::endl;

        std::string reply = "Hello back! Got: " + text;
        Message response(MessageId(service_id_, method_id_),
                         RequestId(msg->get_client_id(), msg->get_session_id()),
                         MessageType::RESPONSE, ReturnCode::E_OK);
        response.set_payload(std::vector<uint8_t>(reply.begin(), reply.end()));
        transport_->send_message(response, sender);
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
    if (argc > 2 && std::string(argv[1]) == "--config") config_path = argv[2];

    examples::Config cfg("sd_demo", config_path);

    auto service_port = cfg.get_uint16("network.server", "service_port", 30500);
    auto multicast_addr = cfg.get_string("network.sd", "multicast_address", "239.255.255.251");
    auto multicast_port = cfg.get_uint16("network.sd", "multicast_port", 30490);
    auto unicast_host = cfg.get_string("network.sd", "unicast_host", "127.0.0.1");
    auto cyclic_offer = cfg.get_int("network.sd", "cyclic_offer_delay_ms", 5000);
    auto ttl = cfg.get_int("network.sd", "ttl", 60);
    auto service_id = cfg.get_uint16("service", "service_id", 0x1000);
    auto instance_id = cfg.get_uint16("service", "instance_id", 0x0001);
    auto method_hello = cfg.get_uint16("service.methods", "hello", 0x0001);

    auto transport = std::make_shared<UdpTransport>(Endpoint("0.0.0.0", service_port));
    RequestHandler handler(transport, service_id, method_hello);
    transport->set_listener(&handler);

    if (transport->start() != Result::SUCCESS) {
        std::cerr << "Failed to start service transport" << std::endl;
        return 1;
    }

    SdConfig sd_cfg;
    sd_cfg.multicast_address = multicast_addr;
    sd_cfg.multicast_port = multicast_port;
    sd_cfg.unicast_address = unicast_host;
    sd_cfg.cyclic_offer = std::chrono::milliseconds(cyclic_offer);

    SdServer sd(sd_cfg);
    if (!sd.initialize()) {
        std::cerr << "Failed to initialise SD server" << std::endl;
        return 1;
    }

    ServiceInstance svc(service_id, instance_id, 1, 0);
    svc.ttl_seconds = ttl;
    std::string unicast_ep = unicast_host + ":" + std::to_string(service_port);
    sd.offer_service(svc, unicast_ep);

    std::cout << "=== SD Demo Server (C++) ===" << std::endl;
    std::cout << "[service] Listening on 0.0.0.0:" << service_port << std::endl;
    std::cout << "[sd] Offering service 0x" << std::hex << service_id
              << " instance 0x" << instance_id << std::dec
              << " at " << unicast_ep << std::endl;
    std::cout << "[sd] Multicast on " << multicast_addr << ":" << multicast_port << std::endl;
    std::cout << "Press Ctrl+C to stop.\n" << std::endl;

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "\nShutting down..." << std::endl;
    sd.shutdown();
    transport->stop();
    std::cout << "Server stopped." << std::endl;
    return 0;
}
