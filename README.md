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

# Filter reports by conditions
./city_manager --role inspector --user John --filter downtown severity:>=:2 category:==:road
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
│   └── report_io.h       # Report CRUD operations
├── src/
│   ├── main.c            # Entry point and command dispatch
│   ├── args.c            # Argument parsing
│   ├── permissions.c     # chmod, stat-based checks, permission string
│   ├── district.c        # District directory and file creation
│   ├── logger.c          # Append timestamped entries to logged_district
│   └── report_io.c       # add, list reports with binary POSIX I/O
├── data/                 # Runtime: district directories (gitignored)
└── tests/                # Test scripts
```
