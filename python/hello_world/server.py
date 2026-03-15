#!/usr/bin/env python3
"""Hello World Server -- simplest SOME/IP request/response over UDP.

Listens for requests to the configured service/method and replies with a
greeting.  Configuration is loaded from ``config/hello_world.yaml``.

Usage::

    python python/hello_world/server.py
    python python/hello_world/server.py --config /path/to/custom.yaml

Cross-language::

    # Python server + C++ client
    python python/hello_world/server.py
    ./cpp/build/bin/hello_world_client
"""

from __future__ import annotations

import signal
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from common.config import add_config_arg, load_config

from opensomeip.message import Message
from opensomeip.transport import Endpoint, UdpTransport
from opensomeip.types import MessageId, MessageType, ReturnCode


def main() -> None:
    parser = add_config_arg()
    args = parser.parse_args()
    cfg = load_config("hello_world", args.config)

    bind_host = cfg["network"]["server"]["host"]
    bind_port = cfg["network"]["server"]["port"]
    service_id = cfg["service"]["service_id"]
    method_id = cfg["service"]["methods"]["say_hello"]

    transport = UdpTransport(local_endpoint=Endpoint(bind_host, bind_port))

    def shutdown(*_: object) -> None:
        print("\nShutting down...")
        transport.stop()

    signal.signal(signal.SIGINT, shutdown)
    signal.signal(signal.SIGTERM, shutdown)

    print("=== SOME/IP Hello World Server (Python) ===")
    print(f"Listening on {bind_host}:{bind_port}")
    print(f"Service 0x{service_id:04X}, method 0x{method_id:04X}")
    print("Press Ctrl+C to exit\n")

    transport.start()

    for msg in transport.receiver:
        if (
            msg.message_id.service_id == service_id
            and msg.message_id.method_id == method_id
            and msg.message_type == MessageType.REQUEST
        ):
            received_text = msg.payload.decode("utf-8", errors="replace")
            print(f"Client said: '{received_text}' (from {msg.source_endpoint})")

            greeting = f"Hello World! Server received: {received_text}"
            response = Message(
                message_id=MessageId(service_id, method_id),
                request_id=msg.request_id,
                message_type=MessageType.RESPONSE,
                return_code=ReturnCode.E_OK,
                payload=greeting.encode("utf-8"),
            )
            transport.send(response, msg.source_endpoint)
            print(f"Sent greeting: '{greeting}'")

    print("Server stopped.")


if __name__ == "__main__":
    main()
