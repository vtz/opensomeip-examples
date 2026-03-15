# opensomeip-examples

Standalone examples showcasing the [opensomeip](https://github.com/vtz/opensomeip) SOME/IP stack in **C++** and **Python**.

Each example is wire-compatible across languages -- you can run a Python server with a C++ client (or vice versa).

## Repository Structure

```
opensomeip-examples/
├── config/            # Shared YAML configuration (read by both Python and C++)
├── python/            # Python examples (uses opensomeip from PyPI)
├── cpp/               # C++ examples (fetches opensomeip via CMake FetchContent)
└── docker/            # Docker Compose for cross-language testing
```

## Examples

| Example | Pattern | Python | C++ | Config |
|---------|---------|--------|-----|--------|
| **hello_world** | Request/Response (UDP) | server, client | server, client | `config/hello_world.yaml` |
| **method_calls** | Calculator RPC | server, client | server, client | `config/method_calls.yaml` |
| **events** | Pub/Sub (sensors) | publisher, subscriber | publisher, subscriber | `config/events.yaml` |
| **sd_demo** | Service Discovery | server | server | `config/sd_demo.yaml` |
| **complex_types** | Serialization | server, client | server, client | `config/complex_types.yaml` |
| **large_messages** | TP segmentation | server, client | server, client | `config/large_messages.yaml` |
| **e2e_protection** | CRC / counter | standalone | standalone | `config/e2e_protection.yaml` |

## Quick Start

### Python

```bash
# Set up a virtual environment (one-time)
bash python/setup_venv.sh
source .venv/bin/activate

# Terminal 1
python python/hello_world/server.py

# Terminal 2
python python/hello_world/client.py
```

You can also set up the venv manually:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r python/requirements.txt
```

### C++

```bash
cd cpp
cmake -B build
cmake --build build -j$(nproc)

# Terminal 1
./build/bin/hello_world_server

# Terminal 2
./build/bin/hello_world_client
```

### Cross-Language (Python server + C++ client)

```bash
# Terminal 1
python python/hello_world/server.py

# Terminal 2
./cpp/build/bin/hello_world_client
```

## Configuration

All examples read from shared YAML config files in `config/`. This ensures both languages agree on ports, service IDs, and other parameters.

Example (`config/hello_world.yaml`):

```yaml
network:
  server:
    host: "0.0.0.0"
    port: 30490
  client:
    server_host: "127.0.0.1"
    server_port: 30490

service:
  service_id: 0x1000
  instance_id: 0x0001
  methods:
    say_hello: 0x0001
```

You can override any value with environment variables using the pattern `OPENSOMEIP_<SECTION>_<KEY>`:

```bash
OPENSOMEIP_SERVER_PORT=31000 python python/hello_world/server.py
```

Or point to a custom config file:

```bash
python python/hello_world/server.py --config /path/to/custom.yaml
```

## Port Map

| Port | Example |
|------|---------|
| 30490 | hello_world, SD multicast |
| 30491 | method_calls |
| 30492 | events publisher |
| 30493 | events subscriber |
| 30494 | complex_types |
| 30495 | large_messages |
| 30500 | sd_demo service |

## License

Apache-2.0. See [LICENSE](LICENSE).
