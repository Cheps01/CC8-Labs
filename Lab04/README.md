# NTP Client

GitHub repo: https://github.com/Cheps01/CC8-Labs.git

A lightweight NTP (Network Time Protocol) client that synchronizes an internal clock with remote NTP servers over UDP.

## Project Structure

```
.
├── main.c              Entry point — parses CLI arguments and runs sync
├── Makefile
├── app/
│   ├── ntp.c / ntp.h   NTP protocol: packet building, parsing, server querying, sync algorithm
│   └── log.c / log.h   Logging to console and per-run log files in logs/
├── network/
│   └── udp.c / udp.h   UDP socket abstraction (send, receive, timeout)
├── time/
│   └── tmp.c / tmp.h   Internal clock: simulated time with offset, get/set functions
└── logs/                Auto-created directory for log files
```

## How It Works

1. **Initialization** — The current system time is read. If CLI arguments provide a custom date/time, an offset is computed between that and the real clock.
2. **Server scan** — All 7 NTP servers are queried to find the one with the lowest round-trip delay.
3. **Sync loop** — The best server is queried repeatedly. Each response's offset is applied to the internal clock until the offset drops below 50 ms.
4. **Logging** — Every run creates a new timestamped log file (e.g. `logs/ntp_sync_20260811_143052.log`) containing both console output and detailed timing data.

## Building

```bash
make
```

Requires `gcc` and a POSIX environment (Linux, WSL, Cygwin).

## Running

```bash
# Sync with system clock
./ntp_client

# Set a custom hour before syncing
./ntp_client -h 14

# Set a full date and time
./ntp_client -Y 2025 -M 1 -D 15 -h 9 -m 30 -s 0

# Only override minutes
./ntp_client -m 45
```

### Flags

| Flag | Meaning       | Example   |
|------|---------------|-----------|
| `-Y` | Year          | `-Y 2025` |
| `-M` | Month (1-12)  | `-M 3`    |
| `-D` | Day (1-31)    | `-D 15`   |
| `-h` | Hour (0-23)   | `-h 14`   |
| `-m` | Minute (0-59) | `-m 30`   |
| `-s` | Second (0-59) | `-s 0`    |

Any omitted flag keeps the current real-time value.

## NTP Servers Queried

- time.google.com
- time.apple.com
- time.windows.com
- time.cloudflare.com
- time.nist.gov
- time-a.nist.gov
- pool.ntp.org
