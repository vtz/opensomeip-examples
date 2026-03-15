# Large Messages

TP (Transport Protocol) segmentation for payloads larger than MTU (2KB - 50KB).

## What it demonstrates

- SOME/IP-TP segmentation for large payloads
- Service 0x5000 with methods: send_large_data, receive_large_data, echo_large_data
- Configurable MTU (default 1400 bytes)
- Payload sizes from 2KB up to 50KB

## How to run

### Python

```bash
# Terminal 1
python python/large_messages/server.py

# Terminal 2
python python/large_messages/client.py
```

### C++

```bash
cd cpp && cmake -B build && cmake --build build

# Terminal 1
./cpp/build/bin/large_messages_server

# Terminal 2
./cpp/build/bin/large_messages_client
```

### Cross-language (Python server + C++ client)

```bash
# Terminal 1
python python/large_messages/server.py

# Terminal 2
./cpp/build/bin/large_messages_client
```

## Configuration

Config file: `config/large_messages.yaml`

Key settings:

- `network.server.port`: 30495
- `service.service_id`: 0x5000
- `tp.mtu`: 1400

Override with env vars: `OPENSOMEIP_<SECTION>_<KEY>`. Use `--config /path/to/custom.yaml` for a custom config file.

## Wire format

Large payloads are segmented using SOME/IP-TP. Each segment carries a 4-byte offset and length. MTU 1400 leaves room for SOME/IP header + TP header + payload per UDP packet.
