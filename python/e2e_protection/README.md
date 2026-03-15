# E2E Protection

CRC computation and E2E protection with corruption detection.

## What it demonstrates

- E2E (End-to-End) protection profile
- CRC16 computation and verification
- Corruption detection on payloads
- Standalone (no network) - runs locally only

## How to run

### Python

```bash
python python/e2e_protection/basic_e2e.py
```

### C++

```bash
cd cpp && cmake -B build && cmake --build build

./cpp/build/bin/basic_e2e
```

### Cross-language

This example is standalone and does not involve network communication. Both Python and C++ implementations demonstrate the same E2E protection logic independently.

## Configuration

Config file: `config/e2e_protection.yaml`

Key settings:

- `e2e.data_id`: 0x1234
- `e2e.profile_id`: 1
- `e2e.crc_type`: crc16
- `test_data`: byte array for CRC computation and corruption tests

Override with env vars: `OPENSOMEIP_<SECTION>_<KEY>`. Use `--config /path/to/custom.yaml` for a custom config file.
