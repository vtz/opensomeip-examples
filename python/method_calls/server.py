#!/usr/bin/env python3
"""Calculator RPC Server -- service with add, multiply, and get_stats methods.

Wire format: big-endian integers.  Configuration from ``config/method_calls.yaml``.
"""

from __future__ import annotations

import signal
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from common.config import add_config_arg, load_config

from opensomeip.message import Message
from opensomeip.transport import Endpoint, UdpTransport
from opensomeip.types import MessageType, ReturnCode

total_calls = 0


def handle_add(payload: bytes) -> bytes:
    global total_calls
    if len(payload) < 8:
        return b""
    a, b = struct.unpack("!ii", payload[:8])
    result = a + b
    total_calls += 1
    print(f"ADD: {a} + {b} = {result}")
    return struct.pack("!i", result)


def handle_multiply(payload: bytes) -> bytes:
    global total_calls
    if len(payload) < 8:
        return b""
    a, b = struct.unpack("!ii", payload[:8])
    result = a * b
    total_calls += 1
    print(f"MULTIPLY: {a} * {b} = {result}")
    return struct.pack("!i", result)


def handle_get_stats(_payload: bytes) -> bytes:
    print(f"GET_STATS: {total_calls} total calls processed")
    return struct.pack("!I", total_calls)


def main() -> None:
    parser = add_config_arg()
    args = parser.parse_args()
    cfg = load_config("method_calls", args.config)

    bind_host = cfg["network"]["server"]["host"]
    bind_port = cfg["network"]["server"]["port"]
    service_id = cfg["service"]["service_id"]
    methods = cfg["service"]["methods"]

    handlers = {
        methods["add"]: handle_add,
        methods["multiply"]: handle_multiply,
        methods["get_stats"]: handle_get_stats,
    }

    transport = UdpTransport(local_endpoint=Endpoint(bind_host, bind_port))

    def shutdown(*_: object) -> None:
        print("\nShutting down...")
        transport.stop()

    signal.signal(signal.SIGINT, shutdown)
    signal.signal(signal.SIGTERM, shutdown)

    print("=== SOME/IP Method Calls Server (Python) ===")
    print(f"Calculator service 0x{service_id:04X} on port {bind_port}")
    print("Available methods:")
    for name, mid in methods.items():
        print(f"  - 0x{mid:04X}: {name}")
    print("Press Ctrl+C to exit\n")

    transport.start()

    for msg in transport.receiver:
        if (
            msg.message_id.service_id == service_id
            and msg.message_type == MessageType.REQUEST
        ):
            handler = handlers.get(msg.message_id.method_id)
            if handler is not None:
                result_payload = handler(msg.payload)
                response = Message(
                    message_id=msg.message_id,
                    request_id=msg.request_id,
                    message_type=MessageType.RESPONSE,
                    return_code=ReturnCode.E_OK,
                    payload=result_payload,
                )
                transport.send(response, msg.source_endpoint)

    print("Server stopped.")


if __name__ == "__main__":
    main()
