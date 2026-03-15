#!/usr/bin/env python3
"""Calculator RPC Client -- calls add, multiply, and get_stats.

Wire format: big-endian integers.  Configuration from ``config/method_calls.yaml``.
"""

from __future__ import annotations

import queue
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from common.config import add_config_arg, load_config

from opensomeip.message import Message
from opensomeip.transport import Endpoint, UdpTransport
from opensomeip.types import MessageId, MessageType, RequestId, ReturnCode

session_counter = 0


def next_session() -> int:
    global session_counter
    session_counter += 1
    return session_counter & 0xFFFF


def call_method(
    transport: UdpTransport,
    server_ep: Endpoint,
    service_id: int,
    method_id: int,
    client_id: int,
    payload: bytes = b"",
    timeout: float = 5.0,
) -> bytes | None:
    request = Message(
        message_id=MessageId(service_id, method_id),
        request_id=RequestId(client_id=client_id, session_id=next_session()),
        message_type=MessageType.REQUEST,
        return_code=ReturnCode.E_OK,
        payload=payload,
    )
    transport.send(request, server_ep)

    try:
        response = transport.receiver._sync_queue.get(timeout=timeout)
        if (
            response.message_id.service_id == service_id
            and response.message_id.method_id == method_id
            and response.message_type == MessageType.RESPONSE
        ):
            return response.payload
    except queue.Empty:
        pass
    return None


def main() -> None:
    parser = add_config_arg()
    args = parser.parse_args()
    cfg = load_config("method_calls", args.config)

    server_host = cfg["network"]["client"]["server_host"]
    server_port = cfg["network"]["client"]["server_port"]
    client_id = cfg["network"]["client"]["client_id"]
    service_id = cfg["service"]["service_id"]
    methods = cfg["service"]["methods"]

    server_ep = Endpoint(server_host, server_port)
    transport = UdpTransport(
        local_endpoint=Endpoint("0.0.0.0", 0),
        remote_endpoint=server_ep,
    )

    print("=== SOME/IP Method Calls Client (Python) ===")
    print(f"Calculator Client (ID: 0x{client_id:04X})\n")

    transport.start()

    def test_add(a: int, b: int) -> None:
        print(f"\n--- Testing ADD({a}, {b}) ---")
        payload = struct.pack("!ii", a, b)
        result = call_method(transport, server_ep, service_id, methods["add"], client_id, payload)
        if result and len(result) >= 4:
            (s,) = struct.unpack("!i", result[:4])
            print(f"Result: {a} + {b} = {s}")
        else:
            print("RPC call failed or timed out")

    def test_multiply(a: int, b: int) -> None:
        print(f"\n--- Testing MULTIPLY({a}, {b}) ---")
        payload = struct.pack("!ii", a, b)
        result = call_method(transport, server_ep, service_id, methods["multiply"], client_id, payload)
        if result and len(result) >= 4:
            (p,) = struct.unpack("!i", result[:4])
            print(f"Result: {a} * {b} = {p}")
        else:
            print("RPC call failed or timed out")

    def test_get_stats() -> None:
        print("\n--- Testing GET_STATS() ---")
        result = call_method(transport, server_ep, service_id, methods["get_stats"], client_id)
        if result and len(result) >= 4:
            (count,) = struct.unpack("!I", result[:4])
            print(f"Server statistics: {count} total method calls processed")
        else:
            print("RPC call failed or timed out")

    print("=== Running Calculator Operations ===")
    test_add(10, 5)
    test_add(-3, 7)
    test_add(1000, 2000)
    test_multiply(6, 7)
    test_multiply(-4, 5)
    test_multiply(25, 4)
    test_get_stats()
    print("\n=== All Operations Completed ===")

    transport.stop()
    print("\nClient finished.")


if __name__ == "__main__":
    main()
