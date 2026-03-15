#!/usr/bin/env python3
"""Complex Types Server -- demonstrates Serializer/Deserializer with structs.

Methods: process_vehicle_data, get_sensor_array, echo_complex_struct.
Configuration from ``config/complex_types.yaml``.
"""

from __future__ import annotations

import signal
import sys
from dataclasses import dataclass
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from common.config import add_config_arg, load_config

from opensomeip.message import Message
from opensomeip.serialization import Deserializer, Serializer
from opensomeip.transport import Endpoint, UdpTransport
from opensomeip.types import MessageType, ReturnCode


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


def deserialize_vehicle(des: Deserializer) -> VehicleData:
    return VehicleData(
        vehicle_id=des.read_uint32(),
        model=des.read_string(),
        fuel_level=des.read_float32(),
        tire_pressure=[des.read_uint8() for _ in range(4)],
        lights_on=des.read_bool(),
        mileage=des.read_uint16(),
    )


def serialize_sensor_reading(reading: SensorReading) -> bytes:
    ser = Serializer()
    ser.write_uint8(reading.sensor_id)
    ser.write_float32(reading.value)
    ser.write_string(reading.unit)
    ser.write_uint32(reading.timestamp)
    return ser.to_bytes()


def deserialize_sensor_reading(des: Deserializer) -> SensorReading:
    return SensorReading(
        sensor_id=des.read_uint8(),
        value=des.read_float32(),
        unit=des.read_string(),
        timestamp=des.read_uint32(),
    )


def serialize_sensor_array(sensors: list[SensorReading]) -> bytes:
    ser = Serializer()
    ser.write_uint32(len(sensors))
    for s in sensors:
        sensor_bytes = serialize_sensor_reading(s)
        ser.write_uint32(len(sensor_bytes))
        ser.write_bytes_raw(sensor_bytes)
    return ser.to_bytes()


def handle_process_vehicle_data(payload: bytes) -> bytes:
    des = Deserializer(payload)
    vehicle = deserialize_vehicle(des)
    print(f"Processing vehicle: {vehicle.model} (ID: {vehicle.vehicle_id})")
    response = f"Processed vehicle data for {vehicle.model} (ID: {vehicle.vehicle_id})"
    ser = Serializer()
    ser.write_string(response)
    return ser.to_bytes()


def handle_get_sensor_array(_payload: bytes) -> bytes:
    sensors = [
        SensorReading(1, 23.5, "C", 1000000),
        SensorReading(2, 65.2, "%", 1000001),
        SensorReading(3, 12.8, "V", 1000002),
        SensorReading(4, 1013.25, "hPa", 1000003),
    ]
    print(f"Returning sensor array with {len(sensors)} readings")
    return serialize_sensor_array(sensors)


def handle_echo_complex_struct(payload: bytes) -> bytes:
    des = Deserializer(payload)
    sensor = deserialize_sensor_reading(des)
    print(f"Echoing sensor: ID={sensor.sensor_id}, Value={sensor.value}{sensor.unit}")
    return serialize_sensor_reading(sensor)


def main() -> None:
    parser = add_config_arg()
    args = parser.parse_args()
    cfg = load_config("complex_types", args.config)

    bind_host = cfg["network"]["server"]["host"]
    bind_port = cfg["network"]["server"]["port"]
    service_id = cfg["service"]["service_id"]
    methods = cfg["service"]["methods"]

    handlers = {
        methods["process_vehicle_data"]: handle_process_vehicle_data,
        methods["get_sensor_array"]: handle_get_sensor_array,
        methods["echo_complex_struct"]: handle_echo_complex_struct,
    }

    transport = UdpTransport(local_endpoint=Endpoint(bind_host, bind_port))

    def shutdown(*_: object) -> None:
        print("\nShutting down...")
        transport.stop()

    signal.signal(signal.SIGINT, shutdown)
    signal.signal(signal.SIGTERM, shutdown)

    print("=== SOME/IP Complex Types Server (Python) ===")
    print(f"Service 0x{service_id:04X} on port {bind_port}")
    for name, mid in methods.items():
        print(f"  0x{mid:04X}: {name}")
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

    transport.stop()
    print("Server stopped.")


if __name__ == "__main__":
    main()
