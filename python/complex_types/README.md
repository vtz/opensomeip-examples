# Complex Types

Serialization with structs (VehicleData, SensorReading) using Serializer/Deserializer.

## What it demonstrates

- Custom struct serialization (VehicleData, SensorReading)
- Service 0x4000 with methods: process_vehicle_data, get_sensor_array, echo_complex_struct
- Serializer/Deserializer for structured payloads

## How to run

### Python

```bash
# Terminal 1
python python/complex_types/server.py

# Terminal 2
python python/complex_types/client.py
```

### C++

```bash
cd cpp && cmake -B build && cmake --build build

# Terminal 1
./cpp/build/bin/complex_types_server

# Terminal 2
./cpp/build/bin/complex_types_client
```

### Cross-language (Python server + C++ client)

```bash
# Terminal 1
python python/complex_types/server.py

# Terminal 2
./cpp/build/bin/complex_types_client
```

## Configuration

Config file: `config/complex_types.yaml`

Key settings:

- `network.server.port`: 30494
- `service.service_id`: 0x4000
- `service.methods`: process_vehicle_data (0x0001), get_sensor_array (0x0002), echo_complex_struct (0x0003)

Override with env vars: `OPENSOMEIP_<SECTION>_<KEY>`. Use `--config /path/to/custom.yaml` for a custom config file.

## Wire format

Payloads use custom struct layouts (VehicleData, SensorReading) serialized in a defined byte order. Both Python and C++ implementations share the same wire format for interoperability.
