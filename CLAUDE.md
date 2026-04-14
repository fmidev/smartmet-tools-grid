# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

`smartmet-tools-grid` — a collection of CLI programs for the SmartMet Server grid support system. These are standalone executables (not a library), organized by purpose: server daemons, API clients, file inspection tools, utilities, and FMI-specific data ingestion programs.

Part of the SmartMet Server ecosystem. See the [parent workspace CLAUDE.md](../CLAUDE.md) for full ecosystem context.

## Build commands

```bash
make                # Build all programs → bin/{clients,files,fmi,servers,utils}/
make debug          # Build with debug flags
make clean          # Remove obj/ and bin/
make rpm            # Build RPM package
make test           # Run tests (currently no test/ directory exists)
make install        # Install to $(PREFIX)/bin/ subdirectories
```

There is no library produced — this project only builds executables. Each `.cpp` file under `src/` compiles to one binary.

CORBA support is enabled by default. Disable with `make CORBA=disabled`.

## Dependencies

Key libraries this project links against (all from the SmartMet ecosystem):
- `grid-files` — GRIB1/GRIB2/NetCDF/QueryData file I/O
- `grid-content` — Content/Data/Query Server APIs (Redis, PostgreSQL, CORBA, HTTP backends)
- `spine` — SmartMet core framework
- `macgyver`, `newbase`, `gis` — foundational SmartMet libraries

External: `libpq`, `hiredis`, `omniORB4`, `libmicrohttpd`, `curl`, `freetype2`, `jpeg`, `png`

## Source layout

```
src/
  servers/     # Standalone server daemons (CORBA/HTTP wrappers around grid-content services)
  clients/     # CLI tools that call Content/Data/Query Server APIs remotely
  files/       # Tools for inspecting/dumping individual grid files
  fmi/         # FMI data ingestion: radon2smartmet, filesys2smartmet, radon2csv, etc.
  utils/       # Assisting programs: Lua execution, geometry queries, PNG merging, etc.
  python/      # Python client scripts for Content Server and visualization
cfg/           # Example configuration files for servers and data ingestion
systemd/       # radon2smartmet.service unit file
```

## Program naming conventions

- `cs_*` — Content Server API client programs
- `ds_*` — Data Server API client programs
- `qs_*` — Query Server API client programs
- `grid_*` — Grid file inspection/manipulation programs
- `gu_*` — Grid utility programs

## How client programs connect to backends

All client programs accept backend selection via trailing command-line flags:
- `-http <url>` — HTTP client
- `-redis <address> <port> <tablePrefix>` — direct Redis access
- `-pg <connectionString>` — PostgreSQL
- `-ior <ior>` — CORBA IOR string
- Default: reads `SMARTMET_CS_IOR` environment variable for CORBA

## Configuration system

Configuration uses libconfig format with SmartMet extensions (`@ifdef`, `@include`, `$(VAR)` expansion). The environment variable `SMARTMET_ENV_FILE` points to a global config that defines paths, addresses, and ports. See `cfg/smartmet-dev-env.cfg` for the development defaults.

Grid file identification requires `SMARTMET_GRID_CONFIG_FILE` environment variable pointing to the grid-files library configuration.

## Key programs

- **radon2smartmet** (`src/fmi/`) — syncs meteorological data from FMI's Radon database to Content Server (Redis/PostgreSQL/HTTP/CORBA). Runs as a daemon via systemd, polling at configurable intervals.
- **filesys2smartmet** (`src/fmi/`) — syncs content information from filesystem-based grid files to Content Server. Alternative to radon2smartmet for non-Radon environments.
- **corbaContentServer / corbaDataServer / corbaQueryServer** (`src/servers/`) — standalone CORBA server wrappers that expose grid-content service APIs.
- **httpContentServer / httpServer** (`src/servers/`) — HTTP server wrappers for content services.
