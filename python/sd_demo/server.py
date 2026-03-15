#!/usr/bin/env python3
"""SD Demo Server -- offers a service via Service Discovery on multicast.

Configuration from ``config/sd_demo.yaml``.
"""

from __future__ import annotations

import signal
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from common.config import add_config_arg, load_config

from opensomeip.message import Message
from opensomeip.sd import SdConfig, SdServer, ServiceInstance
from opensomeip.transport import Endpoint, UdpTransport
from opensomeip.types import MessageId, MessageType, ReturnCode


def main() -> None:
    parser = add_config_arg()
    args = parser.parse_args()
    cfg = load_config("sd_demo", args.config)

    service_port = cfg["network"]["server"]["service_port"]
    sd = cfg["network"]["sd"]
    svc = cfg["service"]

    service_id = svc["service_id"]
    instance_id = svc["instance_id"]
    method_hello = svc["methods"]["hello"]

    transport = UdpTransport(local_endpoint=Endpoint("0.0.0.0", service_port))

    sd_config = SdConfig(
        multicast_endpoint=Endpoint(sd["multicast_address"], sd["multicast_port"]),
        unicast_endpoint=Endpoint(sd["unicast_host"], service_port),
        cyclic_offer_delay_ms=sd["cyclic_offer_delay_ms"],
        ttl=sd["ttl"],
    )
    service = ServiceInstance(
        service_id=service_id,
        instance_id=instance_id,
        major_version=svc.get("major_version", 1),
        minor_version=svc.get("minor_version", 0),
    )

    sd_server = SdServer(sd_config)

    def shutdown(*_: object) -> None:
        print("\nShutting down...")
        sd_server.stop_offer(service)
        sd_server.stop()
        transport.stop()

    signal.signal(signal.SIGINT, shutdown)
    signal.signal(signal.SIGTERM, shutdown)

    transport.start()
    sd_server.start()
    sd_server.offer(service)

    print("=== SD Demo Server (Python) ===")
    print(f"[service] Listening on 0.0.0.0:{service_port}")
    print(
        f"[sd] Offering service 0x{service_id:04X} instance 0x{instance_id:04X} "
        f"at {sd['unicast_host']}:{service_port}"
    )
    print(f"[sd] Multicast on {sd['multicast_address']}:{sd['multicast_port']}")
    print("Press Ctrl+C to stop.\n")

    for msg in transport.receiver:
        if (
            msg.message_id.service_id == service_id
            and msg.message_id.method_id == method_hello
            and msg.message_type == MessageType.REQUEST
        ):
            text = msg.payload.decode("utf-8", errors="replace")
            print(f"[service] Request: '{text}'")

            reply = f"Hello back! Got: {text}"
            response = Message(
                message_id=MessageId(service_id, method_hello),
                request_id=msg.request_id,
                message_type=MessageType.RESPONSE,
                return_code=ReturnCode.E_OK,
                payload=reply.encode("utf-8"),
            )
            transport.send(response, msg.source_endpoint)

    print("Server stopped.")


if __name__ == "__main__":
    main()
