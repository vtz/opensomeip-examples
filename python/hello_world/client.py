#!/usr/bin/env python3
"""Hello World Client -- sends a greeting and prints the server response.

Configuration is loaded from ``config/hello_world.yaml``.

Usage::

    python python/hello_world/client.py
    python python/hello_world/client.py --config /path/to/custom.yaml
"""

from __future__ import annotations

import queue
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from common.config import add_config_arg, load_config

from opensomeip.message import Message
from opensomeip.transport import Endpoint, UdpTransport
from opensomeip.types import MessageId, MessageType, RequestId, ReturnCode


def main() -> None:
    parser = add_config_arg()
    args = parser.parse_args()
    cfg = load_config("hello_world", args.config)

    server_host = cfg["network"]["client"]["server_host"]
    server_port = cfg["network"]["client"]["server_port"]
    service_id = cfg["service"]["service_id"]
    method_id = cfg["service"]["methods"]["say_hello"]

    server_ep = Endpoint(server_host, server_port)
    transport = UdpTransport(
        local_endpoint=Endpoint("0.0.0.0", 0),
        remote_endpoint=server_ep,
    )

    print("=== SOME/IP Hello World Client (Python) ===\n")
    transport.start()

    request = Message(
        message_id=MessageId(service_id, method_id),
        request_id=RequestId(client_id=0x1234, session_id=0x5678),
        message_type=MessageType.REQUEST,
        return_code=ReturnCode.E_OK,
        payload=b"Hello from Python Client!",
    )

    print(f"Sending message: 'Hello from Python Client!' to {server_host}:{server_port}")
    transport.send(request, server_ep)

    print("Waiting for response (5 s timeout)...")
    try:
        response = transport.receiver._sync_queue.get(timeout=5.0)
        if (
            response.message_id.service_id == service_id
            and response.message_id.method_id == method_id
            and response.message_type == MessageType.RESPONSE
        ):
            text = response.payload.decode("utf-8", errors="replace")
            print(f"Server responded: '{text}'")
        else:
            print(f"Unexpected message: {response}")
    except queue.Empty:
        print("Timeout waiting for server response")

    transport.stop()
    print("Client finished.")


if __name__ == "__main__":
    main()
