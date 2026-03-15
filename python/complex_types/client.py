#!/usr/bin/env python3
"""Complex Types Client -- demonstrates Serializer/Deserializer with structs.

Configuration from ``config/complex_types.yaml``.
"""

from __future__ import annotations

import queue
import sys
from dataclasses import dataclass
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from common.config import add_config_arg, load_config

from opensomeip.message import Message
from opensomeip.serialization import Deserializer, Serializer
from opensomeip.transport import Endpoint, UdpTransport
from opensomeip.types import MessageId, MessageType, RequestId, ReturnCode

session_counter = 0


@dataclass
class VehicleData:
    vehicle_id: int
    model: str
    fuel_level: float
    tire_pressure: list[int]
    lights_on: bool
    mileage: int


@dataclass
class SensorReading:
    sensor_id: int
    value: float
    unit: str
    timestamp: int


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
        if response.message_type == MessageType.RESPONSE:
            return response.payload
    except queue.Empty:
        pass
    return None


def serialize_vehicle_data(v: VehicleData) -> bytes:
    ser = Serializer()
    ser.write_uint32(v.vehicle_id)
    ser.write_string(v.model)
    ser.write_float32(v.fuel_level)
    for p in v.tire_pressure:
        ser.write_uint8(p)
    ser.write_bool(v.lights_on)
    ser.write_uint16(v.mileage)
    return ser.to_bytes()


def serialize_sensor_reading(s: SensorReading) -> bytes:
    ser = Serializer()
    ser.write_uint8(s.sensor_id)
    ser.write_float32(s.value)
    ser.write_string(s.unit)
    ser.write_uint32(s.timestamp)
    return ser.to_bytes()


def main() -> None:
    parser = add_config_arg()
    args = parser.parse_args()
    cfg = load_config("complex_types", args.config)

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

    print("=== SOME/IP Complex Types Client (Python) ===")
    print(f"Client ID: 0x{client_id:04X}\n")

    transport.start()

    # Test: Process Vehicle Data
    print("\n--- Test: Process Vehicle Data ---")
    vehicle = VehicleData(42, "Model Y", 0.75, [32, 33, 31, 34], True, 15000)
    payload = serialize_vehicle_data(vehicle)
    result = call_method(transport, server_ep, service_id, methods["process_vehicle_data"], client_id, payload)
    if result:
        des = Deserializer(result)
        print(f"Server response: '{des.read_string()}'")
    else:
        print("RPC call failed or timed out")

    # Test: Get Sensor Array
    print("\n--- Test: Get Sensor Array ---")
    result = call_method(transport, server_ep, service_id, methods["get_sensor_array"], client_id)
    if result:
        des = Deserializer(result)
        count = des.read_uint32()
        print(f"Received {count} sensor readings:")
        for i in range(count):
            _length = des.read_uint32()
            sid = des.read_uint8()
            val = des.read_float32()
            unit = des.read_string()
            ts = des.read_uint32()
            print(f"  [{i}] Sensor {sid}: {val:.1f} {unit} (t={ts})")
    else:
        print("RPC call failed or timed out")

    # Test: Echo Complex Struct
    print("\n--- Test: Echo Complex Struct ---")
    reading = SensorReading(sensor_id=7, value=42.0, unit="dB", timestamp=9999)
    payload = serialize_sensor_reading(reading)
    result = call_method(transport, server_ep, service_id, methods["echo_complex_struct"], client_id, payload)
    if result:
        des = Deserializer(result)
        echo = SensorReading(des.read_uint8(), des.read_float32(), des.read_string(), des.read_uint32())
        print(f"Echoed: sensor_id={echo.sensor_id}, value={echo.value}, unit='{echo.unit}', timestamp={echo.timestamp}")
    else:
        print("RPC call failed or timed out")

    print("\n=== All Demonstrations Completed ===")
    transport.stop()
    print("\nClient finished.")


if __name__ == "__main__":
    main()
