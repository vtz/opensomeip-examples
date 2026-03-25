# opensomeip-examples

Standalone examples showcasing the [opensomeip](https://github.com/vtz/opensomeip) SOME/IP stack in **C++**, **Python**, and **Zephyr RTOS** (with [Renode](https://renode.io/) simulation).

Each example is wire-compatible across languages -- you can run a Python server with a C++ client (or vice versa). The Zephyr examples target embedded platforms and can run on `native_sim` (host simulation) or `s32k388_renode` (NXP S32K388 simulated in Renode).

## Repository Structure

```
opensomeip-examples/
├── config/            # Shared YAML configuration (read by both Python and C++)
├── python/            # Python examples (uses opensomeip from PyPI)
├── cpp/               # C++ examples (fetches opensomeip via CMake FetchContent)
├── zephyr/            # Zephyr RTOS examples (native_sim + S32K388 Renode)
├── scripts/           # Build & test helpers for Zephyr / Renode
└── docker/            # Docker Compose for cross-language and Zephyr testing
```

## Examples

| Example | Pattern | Python | C++ | Zephyr | Config |
|---------|---------|--------|-----|--------|--------|
| **hello_world** | Request/Response (UDP) | server, client | server, client | server, client | `config/hello_world.yaml` |
| **method_calls** | Calculator RPC | server, client | server, client | server, client | `config/method_calls.yaml` |
| **events** | Pub/Sub (sensors) | publisher, subscriber | publisher, subscriber | publisher, subscriber | `config/events.yaml` |
| **sd_demo** | Service Discovery | server | server | -- | `config/sd_demo.yaml` |
| **complex_types** | Serialization | server, client | server, client | -- | `config/complex_types.yaml` |
| **large_messages** | TP segmentation | server, client | server, client | -- | `config/large_messages.yaml` |
| **e2e_protection** | CRC / counter | standalone | standalone | standalone | `config/e2e_protection.yaml` |

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

### Zephyr (native_sim)

```bash
# Prerequisites: Zephyr workspace, opensomeip source
export ZEPHYR_BASE=~/zephyrproject/zephyr
export OPENSOMEIP_ROOT=~/opensomeip

# Build all examples for native_sim
./scripts/zephyr_build.sh native_sim all

# Run the standalone e2e_protection test
./build/zephyr/native_sim_e2e_protection/zephyr/zephyr.exe
```

### Zephyr (S32K388 Renode)

```bash
# Build for the Renode-simulated S32K388
./scripts/zephyr_build.sh s32k388_renode e2e_protection

# Run on Renode
./scripts/run_renode_test.sh e2e_protection
```

### Zephyr via Docker

```bash
# Build and enter the Zephyr dev container
docker compose -f docker/docker-compose.zephyr.yml build
docker compose -f docker/docker-compose.zephyr.yml run zephyr-dev bash

# Inside the container:
./scripts/zephyr_build.sh native_sim all
./scripts/run_renode_test.sh e2e_protection
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

## Zephyr / Renode Targets

The `zephyr/` directory contains Zephyr RTOS versions of the examples, built with [west](https://docs.zephyrproject.org/latest/develop/west/index.html) and the opensomeip Zephyr module.

| Target | Board | Description |
|--------|-------|-------------|
| `native_sim` | Zephyr native simulator | Runs on the host as a Linux process -- ideal for CI and development |
| `s32k388_renode` | NXP S32K388 (Renode) | ARM Cortex-M7 simulated in [Renode](https://renode.io/) with GMAC Ethernet |

### Zephyr examples

| App | SOME/IP Modules | Notes |
|-----|----------------|-------|
| `hello_world_server` | transport (UDP) | Listens for requests, sends responses |
| `hello_world_client` | transport (UDP) | Sends 3 requests, validates responses |
| `method_calls_server` | RPC | Calculator service (add, multiply, stats) |
| `method_calls_client` | RPC | Calls calculator methods, validates results |
| `events_publisher` | events | Publishes temperature and speed events |
| `events_subscriber` | events | Subscribes to sensor event group |
| `e2e_protection` | E2E | Standalone CRC / counter / corruption test |

### Environment variables

| Variable | Description |
|----------|-------------|
| `ZEPHYR_BASE` | Path to the Zephyr RTOS source tree |
| `OPENSOMEIP_ROOT` | Path to the opensomeip source tree |
| `RENODE_TIMEOUT` | Renode simulation timeout in seconds (default: 15) |

## License

Apache-2.0. See [LICENSE](LICENSE).
