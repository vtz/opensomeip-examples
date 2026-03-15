# SD Demo

Service Discovery with multicast offer and request handling.

## What it demonstrates

- SOME/IP Service Discovery (SD) using multicast
- Service 0x1000, instance 0x0001 offered via SD
- Cyclic offer with configurable TTL and delay
- Handles SOME/IP requests on the service transport

## How to run

### Python

```bash
python python/sd_demo/server.py
```

### C++

```bash
cd cpp && cmake -B build && cmake --build build
./cpp/build/bin/sd_demo_server
```

## Configuration

Config file: `config/sd_demo.yaml`

Key settings:

- `network.server.service_port`: 30500
- `network.sd.multicast_address`: 239.255.255.251
- `network.sd.multicast_port`: 30490
- `network.sd.unicast_host`: 127.0.0.1
- `network.sd.cyclic_offer_delay_ms`: 5000
- `network.sd.ttl`: 60
- `service.service_id`: 0x1000
- `service.instance_id`: 0x0001

Override with env vars: `OPENSOMEIP_<SECTION>_<KEY>`. Use `--config /path/to/custom.yaml` for a custom config file.

## Notes

This example only includes a server. Any SD-aware client (Python, C++, or external) can discover and communicate with the offered service via multicast.
