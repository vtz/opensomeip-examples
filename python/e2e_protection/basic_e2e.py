#!/usr/bin/env python3
"""Basic E2E Protection -- standalone CRC / counter / corruption detection demo.

No transport is used -- this demonstrates E2E protection concepts.
Configuration from ``config/e2e_protection.yaml``.
"""

from __future__ import annotations

import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from common.config import add_config_arg, load_config

from opensomeip.e2e import (
    E2ECheckStatus,
    E2EConfig,
    E2EProfile,
    E2EProfileId,
    E2EProtection,
    crc8,
    crc16,
    crc32,
)


class SimpleE2EProfile(E2EProfile):
    """A simple E2E profile that prepends CRC-16 + counter to the payload."""

    @property
    def profile_id(self) -> int:
        return E2EProfileId.PROFILE_01

    @property
    def header_size(self) -> int:
        return 4

    def protect(self, data: bytearray, counter: int) -> bytearray:
        crc = crc16(bytes(data))
        header = struct.pack("!HBB", crc, counter & 0xFF, 0x00)
        return bytearray(header) + data

    def check(self, data: bytes, counter: int) -> E2ECheckStatus:
        if len(data) < 4:
            return E2ECheckStatus.ERROR
        stored_crc, stored_counter, _ = struct.unpack("!HBB", data[:4])
        payload = data[4:]
        computed_crc = crc16(payload)
        if computed_crc != stored_crc:
            return E2ECheckStatus.ERROR
        if stored_counter != (counter & 0xFF):
            return E2ECheckStatus.WRONG_SEQUENCE
        return E2ECheckStatus.OK


def main() -> None:
    parser = add_config_arg()
    args = parser.parse_args()
    cfg = load_config("e2e_protection", args.config)

    data_id = cfg["e2e"]["data_id"]
    test_data = bytes(cfg["test_data"])

    print("=== SOME/IP Basic E2E Protection (Python) ===\n")

    # CRC demonstrations
    print("--- CRC Functions ---")
    print(f"Data: {test_data.hex()}")
    print(f"  CRC-8 (SAE-J1850): 0x{crc8(test_data):02X}")
    print(f"  CRC-16 (ITU-T):    0x{crc16(test_data):04X}")
    print(f"  CRC-32:            0x{crc32(test_data):08X}")

    # Sender/receiver E2E protection
    print("\n--- E2E Sender / Receiver ---")
    config = E2EConfig(
        profile_id=E2EProfileId.PROFILE_01,
        data_id=data_id,
        data_length=len(test_data),
    )
    profile = SimpleE2EProfile()

    sender = E2EProtection(config, profile=profile)
    receiver = E2EProtection(config, profile=profile)

    for i in range(3):
        protected = sender.protect(test_data)
        print(f"\n  Message {i}: {protected.hex()}")
        print(f"    Header: CRC=0x{protected[0]:02X}{protected[1]:02X} counter={protected[2]}")
        status = receiver.check(protected)
        print(f"    Check:  {status.name}")
        receiver._counter = (receiver._counter + 1) & 0xFF

    # Corruption detection
    print("\n--- Corruption Detection ---")
    protected = sender.protect(test_data)
    print(f"  Protected: {protected.hex()}")

    corrupted = bytearray(protected)
    corrupted[5] ^= 0xFF
    status = receiver.check(bytes(corrupted))
    print(f"  Corrupted payload -> {status.name}")

    corrupted2 = bytearray(protected)
    corrupted2[0] ^= 0xFF
    status2 = receiver.check(bytes(corrupted2))
    print(f"  Corrupted CRC     -> {status2.name}")

    print("\n=== E2E Protection Demo Complete ===")


if __name__ == "__main__":
    main()
