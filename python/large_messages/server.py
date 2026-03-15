#!/usr/bin/env python3
"""Large Messages Server -- demonstrates TP segmentation for large payloads.

Methods: send_large_data, receive_large_data, echo_large_data.
Configuration from ``config/large_messages.yaml``.
"""

from __future__ import annotations

import signal
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from common.config import add_config_arg, load_config

from opensomeip.message import Message
from opensomeip.tp import TpManager
from opensomeip.transport import Endpoint, UdpTransport
from opensomeip.types import MessageType, ReturnCode


def generate_test_data(size: int) -> bytes:
    pattern = bytes(range(256))
    full, remainder = divmod(size, 256)
    return pattern * full + pattern[:remainder]


def verify_test_data(data: bytes) -> bool:
    return all(byte == i % 256 for i, byte in enumerate(data))


def handle_send_large_data(payload: bytes) -> bytes:
    if len(payload) < 4:
        return b""
    (size,) = struct.unpack("!I", payload[:4])
    print(f"SEND_LARGE_DATA: Generating {size} bytes of test data")
    return generate_test_data(size)


def handle_receive_large_data(payload: bytes) -> bytes:
    ok = verify_test_data(payload)
    status = "valid" if ok else "CORRUPTED"
    print(f"RECEIVE_LARGE_DATA: Received {len(payload)} bytes -- {status}")
    response = f"Received {len(payload)} bytes, integrity: {status}"
    return response.encode("utf-8")


def handle_echo_large_data(payload: bytes) -> bytes:
    print(f"ECHO_LARGE_DATA: Echoing {len(payload)} bytes")
    return payload


def main() -> None:
    parser = add_config_arg()
    args = parser.parse_args()
    cfg = load_config("large_messages", args.config)

    bind_host = cfg["network"]["server"]["host"]
    bind_port = cfg["network"]["server"]["port"]
    service_id = cfg["service"]["service_id"]
    methods = cfg["service"]["methods"]
    mtu = cfg["tp"]["mtu"]

    handlers = {
        methods["send_large_data"]: handle_send_large_data,
        methods["receive_large_data"]: handle_receive_large_data,
        methods["echo_large_data"]: handle_echo_large_data,
    }

    transport = UdpTransport(local_endpoint=Endpoint(bind_host, bind_port))
    tp = TpManager(transport=transport, mtu=mtu)

    def shutdown(*_: object) -> None:
        print("\nShutting down...")
        tp.stop()
        transport.stop()

    signal.signal(signal.SIGINT, shutdown)
    signal.signal(signal.SIGTERM, shutdown)

    print("=== SOME/IP Large Messages Server (Python) ===")
    print(f"Service 0x{service_id:04X} on port {bind_port} (MTU={mtu})")
    for name, mid in methods.items():
        print(f"  0x{mid:04X}: {name}")
    print("Press Ctrl+C to exit\n")

    transport.start()
    tp.start()

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
                tp.send(response, endpoint=msg.source_endpoint)

    tp.stop()
    transport.stop()
    print("Server stopped.")


if __name__ == "__main__":
    main()
