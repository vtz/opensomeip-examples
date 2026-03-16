#!/usr/bin/env python3
"""Sensor Event Publisher -- publishes temperature and speed events.

Payload is a big-endian IEEE 754 float (4 bytes).
Configuration from ``config/events.yaml``.

Usage::

    python python/events/publisher.py
    python python/events/publisher.py --config /path/to/custom.yaml

Cross-language::

    # Python publisher + C++ subscriber
    python python/events/publisher.py
    ./cpp/build/bin/events_subscriber
"""

from __future__ import annotations

import random
import signal
import struct
import sys
import threading
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from common.config import add_config_arg, load_config

from opensomeip.message import Message
from opensomeip.transport import Endpoint, UdpTransport
from opensomeip.types import MessageId, MessageType, ReturnCode

stop_event = threading.Event()


def float_to_be_bytes(value: float) -> bytes:
    return struct.pack("!f", value)


def main() -> None:
    parser = add_config_arg()
    args = parser.parse_args()
    cfg = load_config("events", args.config)

    port = cfg["network"]["publisher"]["port"]
    subscriber_host = cfg["network"]["publisher"]["subscriber_host"]
    subscriber_port = cfg["network"]["publisher"]["subscriber_port"]
    service_id = cfg["service"]["service_id"]
    temp_event = cfg["service"]["events"]["temperature"]
    speed_event = cfg["service"]["events"]["speed"]
    temp_interval = cfg["timing"]["temperature_interval_s"]
    speed_interval = cfg["timing"]["speed_interval_s"]

    signal.signal(signal.SIGINT, lambda *_: stop_event.set())
    signal.signal(signal.SIGTERM, lambda *_: stop_event.set())

    subscriber_ep = Endpoint(subscriber_host, subscriber_port)
    transport = UdpTransport(local_endpoint=Endpoint("0.0.0.0", port))

    transport.start()

    print("=== SOME/IP Events Publisher (Python) ===")
    print(f"Service 0x{service_id:04X} on port {port}")
    print(f"  Temperature (0x{temp_event:04X}) every {temp_interval} s")
    print(f"  Speed       (0x{speed_event:04X}) every {speed_interval} s")
    print(f"  Sending to {subscriber_host}:{subscriber_port}")
    print("Press Ctrl+C to exit\n")

    last_temp = time.monotonic()
    last_speed = time.monotonic()

    while not stop_event.is_set():
        now = time.monotonic()

        if now - last_temp >= temp_interval:
            temperature = random.uniform(15.0, 35.0)
            msg = Message(
                message_id=MessageId(service_id, temp_event),
                message_type=MessageType.NOTIFICATION,
                return_code=ReturnCode.E_OK,
                payload=float_to_be_bytes(temperature),
            )
            transport.send(msg, subscriber_ep)
            print(f"Published Temperature: {temperature:.1f} C")
            last_temp = now

        if now - last_speed >= speed_interval:
            speed = random.uniform(0.0, 120.0)
            msg = Message(
                message_id=MessageId(service_id, speed_event),
                message_type=MessageType.NOTIFICATION,
                return_code=ReturnCode.E_OK,
                payload=float_to_be_bytes(speed),
            )
            transport.send(msg, subscriber_ep)
            print(f"Published Speed: {speed:.1f} km/h")
            last_speed = now

        stop_event.wait(0.1)

    print("\nShutting down...")
    transport.stop()
    print("Publisher stopped.")


if __name__ == "__main__":
    main()
