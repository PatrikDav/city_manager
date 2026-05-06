# City Manager - Infrastructure Issue Reporting System

A C program for UNIX that manages city infrastructure issue reports. City inspectors file structured reports about problems found across urban districts (damaged roads, broken lighting, flooding, etc.). Managers oversee and manage these reports.

## Build Requirements

- **Linux environment**: WSL (Ubuntu), Debian, or native Linux
- **GCC compiler**: `sudo apt install build-essential`
- **POSIX system**: Required for `lstat()`, `symlink()`, `chmod()`, `ftruncate()`

## Build

```bash
make          # Compile the project
make clean    # Remove build artifacts
make rebuild  # Clean + compile
```

## Usage

```bash
./city_manager --role <inspector|manager> --user <name> <command>
```

### Commands

| Command | Arguments | Who can use it |
|---------|-----------|---------------|
| `--add` | `<district>` | Both roles |
| `--list` | `<district>` | Both roles |
| `--view` | `<district> <report_id>` | Both roles |
| `--remove_report` | `<district> <report_id>` | Manager only |
| `--update_threshold` | `<district> <value>` | Manager only |
| `--filter` | `<district> <conditions...>` | Both roles |
| `--remove_district` | `<district>` | Manager only |

### Examples

```bash
# Add a report as an inspector
./city_manager --role inspector --user John --add downtown

# List all reports in a district
./city_manager --role inspector --user John --list downtown

# View a specific report
./city_manager --role inspector --user John --view downtown 1

# Remove a report (manager only)
./city_manager --role manager --user Alice --remove_report downtown 3

# Update severity threshold (manager only)
./city_manager --role manager --user Alice --update_threshold downtown 3

# Filter reports by conditions (quote operators that contain > or <)
./city_manager --role inspector --user John --filter downtown 'severity:>=:2' 'category:==:road'

# Remove an entire district (manager only)
./city_manager --role manager --user Alice --remove_district downtown
```

## monitor_reports

A companion background process that listens for new report events.

```bash
# Start in background (run from project root)
./monitor_reports &

# It writes its PID to data/.monitor_pid
# When city_manager adds a report, it sends SIGUSR1 to the monitor
# The monitor prints a message to stdout for each SIGUSR1 received
# Stop it cleanly with Ctrl+C (SIGINT) or:
kill -INT $(cat data/.monitor_pid)
```

## Project Structure

```
city_manager/
├── Makefile
├── include/
│   ├── report.h          # Report struct (fixed-size binary record)
│   ├── args.h            # Role/Command enums, Args struct
│   ├── permissions.h     # Permission constants and enforcement
│   ├── district.h        # District directory initialization
│   ├── logger.h          # Operation logging
│   ├── report_io.h       # Report CRUD operations
│   ├── symlinks.h        # Symlink create and dangling detection
│   └── filter.h          # Condition struct, parse/match/filter functions
├── src/
│   ├── main.c              # Entry point and command dispatch
│   ├── args.c              # Argument parsing
│   ├── permissions.c       # chmod, stat-based checks, permission string
│   ├── district.c          # District directory and file creation
│   ├── logger.c            # Append timestamped entries to logged_district
│   ├── report_io.c         # add, list, view, remove, update_threshold
│   ├── symlinks.c          # symlink(), lstat(), resolve_or_warn()
│   ├── filter.c            # parse_condition, match_condition
│   ├── remove_district.c   # fork+execvp rm -rf, symlink unlink
│   ├── monitor_notify.c    # read .monitor_pid, send SIGUSR1
│   └── monitor_reports.c   # standalone monitor binary
├── data/                   # Runtime: district directories (gitignored)
├── tests/
│   └── test_commands.sh    # End-to-end test script
└── AI_usage-phases_1_and_2.md  # AI-assisted function documentation
```
