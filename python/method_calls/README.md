# Method Calls

Calculator RPC with add, multiply, and get_stats methods.

## What it demonstrates

- Remote procedure calls with input/output parameters
- Service 0x2000 with methods: add (0x0001), multiply (0x0002), get_stats (0x0003)
- Big-endian integer serialization for RPC payloads

## How to run

### Python

```bash
# Terminal 1
python python/method_calls/server.py

# Terminal 2
python python/method_calls/client.py
```

### C++

```bash
cd cpp && cmake -B build && cmake --build build

# Terminal 1
./cpp/build/bin/method_calls_server

# Terminal 2
./cpp/build/bin/method_calls_client
```

### Cross-language (Python server + C++ client)

```bash
# Terminal 1
python python/method_calls/server.py

# Terminal 2
./cpp/build/bin/method_calls_client
```

## Configuration

Config file: `config/method_calls.yaml`

Key settings:

- `network.server.port`: 30491
- `network.client.client_id`: 0xABCD
- `service.service_id`: 0x2000
- `service.methods`: add (0x0001), multiply (0x0002), get_stats (0x0003)

Override with env vars: `OPENSOMEIP_<SECTION>_<KEY>`. Use `--config /path/to/custom.yaml` for a custom config file.

## Wire format

Parameters and results are big-endian int32 values. ADD and MULTIPLY take two int32 inputs and return one int32. GET_STATS returns a uint32 call count.
