# Events

Publish/subscribe pattern with temperature and speed sensor events.

## What it demonstrates

- Event publishing and subscription
- Service 0x3000 with events: temperature (0x8001), speed (0x8002)
- Eventgroup-based subscription (eventgroup 0x0001)
- Periodic event publishing with configurable intervals

## How to run

### Python

```bash
# Terminal 1
python python/events/publisher.py

# Terminal 2
python python/events/subscriber.py
```

### C++

```bash
cd cpp && cmake -B build && cmake --build build

# Terminal 1
./cpp/build/bin/events_publisher

# Terminal 2
./cpp/build/bin/events_subscriber
```

### Cross-language (Python publisher + C++ subscriber)

```bash
# Terminal 1
python python/events/publisher.py

# Terminal 2
./cpp/build/bin/events_subscriber
```

## Configuration

Config file: `config/events.yaml`

Key settings:

- `network.publisher.port`: 30492
- `network.subscriber.port`: 30493
- `service.service_id`: 0x3000
- `service.events`: temperature (0x8001), speed (0x8002)
- `timing.temperature_interval_s`: 2.0
- `timing.speed_interval_s`: 1.5

Override with env vars: `OPENSOMEIP_<SECTION>_<KEY>`. Use `--config /path/to/custom.yaml` for a custom config file.

## Wire format

Event payloads are 4-byte big-endian IEEE 754 floats. Temperature is in degrees Celsius, speed in km/h.
