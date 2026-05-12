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
make install        # Install to $(PREFIX)/bin/ subdirectories
```

There is no library produced — each `.cpp` file under `src/` compiles to exactly one independent binary. The Makefile uses `$(wildcard src/*/*.cpp)` to discover all sources automatically, so adding a new `.cpp` is sufficient to create a new binary.

CORBA support is enabled by default. Disable with `make CORBA=disabled`.

## Environment setup for running tools

Most tools require environment variables before they will work:

```bash
# Required for all grid_* file inspection tools
export SMARTMET_GRID_CONFIG_FILE=~/workspace/smartmet/smartmet-library-grid-files/cfg/grid-files.conf

# Required for client tools that default to CORBA backend
export SMARTMET_CS_IOR="IOR:..."   # CORBA IOR for Content Server

# Optional: point to global dev env config (used by filesys2smartmet, radon2smartmet)
export SMARTMET_ENV_FILE=~/workspace/smartmet/smartmet-tools-grid/cfg/smartmet-dev-env.cfg
```

`cfg/smartmet-dev-env.cfg` defines all dev-environment addresses (Redis, CORBA, HTTP, PostgreSQL endpoints) and is included by libconfig configs via `@include "$(SMARTMET_ENV_FILE)"`.

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
  clients/     # CLI tools that call Content/Data/Query Server APIs remotely (~155 programs)
  files/       # Tools for inspecting/dumping individual grid files
  fmi/         # FMI data ingestion: radon2smartmet, filesys2smartmet, radon2csv, etc.
  utils/       # Assisting programs: Lua execution, geometry queries, PNG merging, etc.
  python/      # Python client scripts for Content Server and visualization
cfg/           # Example configuration files for servers and data ingestion
doc/           # Full documentation (md, pdf) and man pages (doc/man/)
systemd/       # radon2smartmet.service unit file
```

## Program naming conventions

- `cs_*` — Content Server API client programs (metadata: producers, generations, files, content)
- `ds_*` — Data Server API client programs (actual grid data retrieval)
- `qs_*` — Query Server API client programs (parameterized grid queries)
- `grid_*` — Grid file inspection/manipulation programs
- `gu_*` — Grid utility programs

## Three-tier grid API

The grid subsystem exposes three layered service APIs, each with its own client programs:

- **Content Server** (`cs_*`) — metadata store: producers, generations (forecast runs), file registrations, content records. Backends: Redis, PostgreSQL, CORBA, HTTP.
- **Data Server** (`ds_*`) — retrieves actual grid data values from registered files; sits above Content Server.
- **Query Server** (`qs_*`) — high-level parameterized queries (by parameter name, level, time, geometry); sits above both.

Each API has CORBA, HTTP, Redis (CS only), and PostgreSQL (CS only) implementations selectable at runtime.

## How client programs connect to backends

All client programs accept backend selection via trailing command-line flags:
- `-http <url>` — HTTP client
- `-redis <address> <port> <tablePrefix>` — direct Redis access
- `-pg <connectionString>` — PostgreSQL
- `-ior <ior>` — CORBA IOR string
- Default: reads `SMARTMET_CS_IOR` environment variable for CORBA

## Code pattern for new programs

**Client programs** (`cs_*`, `ds_*`, `qs_*`): parse positional args, detect backend from trailing flags, instantiate the matching `*Implementation` class, call the service method, print results.

```cpp
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
// ... other backends
using namespace SmartMet;

int main(int argc, char *argv[]) {
  ContentServer::ServiceInterface *service = nullptr;
  if (strcmp(argv[argc-2], "-http") == 0) { /* httpClient->init(...) */ }
  else if (strcmp(argv[argc-4], "-redis") == 0) { /* redis->init(...) */ }
  // ... pg, ior, default CORBA from env
  service->someMethod(...);
}
```

**File tools** (`grid_*`): call `Identification::gridDef.init(configFile)` from `SMARTMET_GRID_CONFIG_FILE`, then open `GRID::GridFile`, iterate messages.

```cpp
#include "grid-files/grid/GridFile.h"
#include "grid-files/identification/GridDef.h"
// init: Identification::gridDef.init(getenv(SMARTMET_GRID_CONFIG_FILE));
// use:  GRID::GridFile gridFile; gridFile.read(filename);
```

Exception handling always uses:
```cpp
throw Fmi::Exception(BCP, "message", nullptr);   // BCP = __FILE__,__LINE__,__FUNCTION__
// or in catch blocks:
Fmi::Exception::Trace(BCP, "message");
```

## Configuration system

Configuration uses libconfig format with SmartMet extensions (`@ifdef`, `@include`, `$(VAR)` expansion). The environment variable `SMARTMET_ENV_FILE` points to a global config that defines paths, addresses, and ports. See `cfg/smartmet-dev-env.cfg` for the development defaults.

Grid file identification requires `SMARTMET_GRID_CONFIG_FILE` environment variable pointing to the grid-files library configuration.

### filesys2smartmet configuration files

`filesys2smartmet` is configured via three complementary files:
- Main libconfig file (e.g., `cfg/filesys-to-smartmet.cfg`) — storage backend, polling interval, scan paths
- `producerDef.csv` — maps filename patterns (regex) to producer IDs and metadata
- `filenameFixer.lua` (optional) — Lua script for transforming filenames before pattern matching

## Key programs

- **radon2smartmet** (`src/fmi/`) — syncs meteorological data from FMI's Radon database to Content Server (Redis/PostgreSQL/HTTP/CORBA). Runs as a daemon via systemd, polling at configurable intervals.
- **filesys2smartmet** (`src/fmi/`) — syncs content information from filesystem-based grid files to Content Server. Alternative to radon2smartmet for non-Radon environments.
- **corbaContentServer / corbaDataServer / corbaQueryServer** (`src/servers/`) — standalone CORBA server wrappers that expose grid-content service APIs.
- **httpContentServer / httpServer** (`src/servers/`) — HTTP server wrappers for content services.
- **grid_dump / grid_info** (`src/files/`) — inspect GRIB/NetCDF/QueryData file structure and content without a running server.
