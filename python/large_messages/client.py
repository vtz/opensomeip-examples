#!/usr/bin/env python3
"""Large Messages Client -- tests large data transfers with TP segmentation.

Configuration from ``config/large_messages.yaml``.
"""

from __future__ import annotations

import queue
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from common.config import add_config_arg, load_config

from opensomeip.message import Message
from opensomeip.tp import TpManager
from opensomeip.transport import Endpoint, UdpTransport
from opensomeip.types import MessageId, MessageType, RequestId, ReturnCode

session_counter = 0


def next_session() -> int:
    global session_counter
    session_counter += 1
    return session_counter & 0xFFFF


def generate_test_data(size: int) -> bytes:
    pattern = bytes(range(256))
    full, remainder = divmod(size, 256)
    return pattern * full + pattern[:remainder]


def call_method(
    tp: TpManager,
    transport: UdpTransport,
    service_id: int,
    method_id: int,
    client_id: int,
    payload: bytes = b"",
    timeout: float = 10.0,
) -> bytes | None:
    request = Message(
        message_id=MessageId(service_id, method_id),
        request_id=RequestId(client_id=client_id, session_id=next_session()),
        message_type=MessageType.REQUEST,
        return_code=ReturnCode.E_OK,
        payload=payload,
    )
    tp.send(request)
    try:
        response = transport.receiver._sync_queue.get(timeout=timeout)
        if response.message_type == MessageType.RESPONSE:
            return response.payload
    except queue.Empty:
        pass
    return None


def main() -> None:
    parser = add_config_arg()
    args = parser.parse_args()
    cfg = load_config("large_messages", args.config)

    server_host = cfg["network"]["client"]["server_host"]
    server_port = cfg["network"]["client"]["server_port"]
    client_id = cfg["network"]["client"]["client_id"]
    service_id = cfg["service"]["service_id"]
    methods = cfg["service"]["methods"]
    mtu = cfg["tp"]["mtu"]

    server_ep = Endpoint(server_host, server_port)
    transport = UdpTransport(
        local_endpoint=Endpoint("0.0.0.0", 0),
        remote_endpoint=server_ep,
    )
    tp = TpManager(transport=transport, mtu=mtu)

    print("=== SOME/IP Large Messages Client (Python) ===")
    print(f"Client ID: 0x{client_id:04X}\n")

    transport.start()
    tp.start()

    for size in (2048, 10240, 51200):
        print(f"\n--- Requesting {size:,} bytes ---")
        payload = struct.pack("!I", size)
        result = call_method(tp, transport, service_id, methods["send_large_data"], client_id, payload)
        if result:
            print(f"Received {len(result):,} bytes (expected {size:,})")
            print(f"Size check: {'PASS' if len(result) == size else 'FAIL'}")
        else:
            print("Request timed out")

    print(f"\n--- Sending 10,240 bytes for verification ---")
    data = generate_test_data(10240)
    result = call_method(tp, transport, service_id, methods["receive_large_data"], client_id, data)
    if result:
        print(f"Server response: '{result.decode('utf-8', errors='replace')}'")
    else:
        print("Request timed out")

    print(f"\n--- Echo round-trip: 15,360 bytes ---")
    data = generate_test_data(15360)
    result = call_method(tp, transport, service_id, methods["echo_large_data"], client_id, data)
    if result:
        ok = result == data
        print(f"Echo {len(result):,} bytes -- {'PASS' if ok else 'MISMATCH'}")
    else:
        print("Request timed out")

    print("\n=== All Tests Completed ===")

    tp.stop()
    transport.stop()
    print("\nClient finished.")


if __name__ == "__main__":
    main()
