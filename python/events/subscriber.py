#!/usr/bin/env python3
"""Sensor Event Subscriber -- receives temperature and speed events.

Decodes 4-byte big-endian IEEE 754 float payload.
Configuration from ``config/events.yaml``.

Usage::

    python python/events/subscriber.py
    python python/events/subscriber.py --config /path/to/custom.yaml

Cross-language::

    # C++ publisher + Python subscriber
    ./cpp/build/bin/events_publisher
    python python/events/subscriber.py
"""

from __future__ import annotations

import signal
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from common.config import add_config_arg, load_config

from opensomeip.transport import Endpoint, UdpTransport
from opensomeip.types import MessageType


def be_bytes_to_float(data: bytes) -> float:
    (value,) = struct.unpack("!f", data[:4])
    return value


def main() -> None:
    parser = add_config_arg()
    args = parser.parse_args()
    cfg = load_config("events", args.config)

    port = cfg["network"]["subscriber"]["port"]
    service_id = cfg["service"]["service_id"]
    temp_event = cfg["service"]["events"]["temperature"]
    speed_event = cfg["service"]["events"]["speed"]

    transport = UdpTransport(local_endpoint=Endpoint("0.0.0.0", port))

    def shutdown(*_: object) -> None:
        print("\nShutting down...")
        transport.stop()

    signal.signal(signal.SIGINT, shutdown)
    signal.signal(signal.SIGTERM, shutdown)

    transport.start()

    print("=== SOME/IP Events Subscriber (Python) ===")
    print(f"Service 0x{service_id:04X}, listening on port {port}")
    print(f"  Temperature (0x{temp_event:04X})")
    print(f"  Speed       (0x{speed_event:04X})")
    print("Waiting for events... Press Ctrl+C to exit\n")

    for msg in transport.receiver:
        if msg.message_type != MessageType.NOTIFICATION:
            continue
        if msg.message_id.service_id != service_id:
            continue

        event_id = msg.message_id.method_id
        if len(msg.payload) < 4:
            print(f"Event 0x{event_id:04X}: Invalid data size ({len(msg.payload)} bytes)")
            continue

        value = be_bytes_to_float(msg.payload)

        if event_id == temp_event:
            print(f"Temperature Event: {value:.1f} C")
        elif event_id == speed_event:
            print(f"Speed Event: {value:.1f} km/h")
        else:
            print(f"Unknown event 0x{event_id:04X}: {msg.payload.hex()}")

    print("Subscriber stopped.")


if __name__ == "__main__":
    main()
