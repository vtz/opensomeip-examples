# Hello World

Simplest SOME/IP request/response over UDP.

## What it demonstrates

- Basic SOME/IP message exchange (request + response)
- Service 0x1000 with method say_hello (0x0001)
- UdpTransport usage and message construction

## How to run

### Python

```bash
# Terminal 1
python python/hello_world/server.py

# Terminal 2
python python/hello_world/client.py
```

### C++

```bash
cd cpp && cmake -B build && cmake --build build

# Terminal 1
./cpp/build/bin/hello_world_server

# Terminal 2
./cpp/build/bin/hello_world_client
```

### Cross-language (Python server + C++ client)

```bash
# Terminal 1
python python/hello_world/server.py

# Terminal 2
./cpp/build/bin/hello_world_client
```

## Configuration

Config file: `config/hello_world.yaml`

Key settings:

- `network.server.host`: bind address (default 0.0.0.0)
- `network.server.port`: 30490
- `network.client.server_host`: 127.0.0.1
- `service.service_id`: 0x1000
- `service.methods.say_hello`: 0x0001

Override with env vars: `OPENSOMEIP_<SECTION>_<KEY>`. Use `--config /path/to/custom.yaml` for a custom config file.

## Wire format

UTF-8 encoded string payload. Server echoes back a greeting that includes the received text.
