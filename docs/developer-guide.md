# tools-grid developer guide

This guide is for developers who change `smartmet-tools-grid` or use its programs to
run, feed or debug the SmartMet grid services. It covers what the programs are, how the
two ingest programs work, the standalone servers, the command-line clients, and the
pitfalls.

The programs are thin wrappers around the grid libraries. See the developer guides of
[grid-files](https://github.com/fmidev/smartmet-library-grid-files/blob/master/docs/developer-guide.md)
(file reading and identification),
[grid-content](https://github.com/fmidev/smartmet-library-grid-content/blob/master/docs/developer-guide.md)
(the Content, Data and Query Servers, the content model, events, the Redis layout) and
the [grid engine](https://github.com/fmidev/smartmet-engine-grid/blob/master/docs/developer-guide.md).
User documentation is in [doc/](../doc/): `grid-support.md` (overview),
`filesys2smartmet.md` (its configuration), and man pages in `doc/man/`.

## Contents

1. [What is in this repository](#1-what-is-in-this-repository)
2. [Building and installing](#2-building-and-installing)
3. [Environment](#3-environment)
4. [filesys2smartmet](#4-filesys2smartmet)
5. [radon2smartmet](#5-radon2smartmet)
6. [Other FMI programs](#6-other-fmi-programs)
7. [Standalone servers](#7-standalone-servers)
8. [Client programs](#8-client-programs)
9. [File and utility programs](#9-file-and-utility-programs)
10. [Recipes](#10-recipes)
11. [Common tasks](#11-common-tasks)
12. [Known pitfalls](#12-known-pitfalls)

---

## 1. What is in this repository

Every `.cpp` file under `src/` is one independent program:

| Directory | Programs | Purpose |
|-----------|----------|---------|
| `src/fmi/` | `filesys2smartmet`, `radon2smartmet`, `radon2config`, `radon2csv`, `createLandSeaMap` | **Ingest**: write producers, generations, geometries, files and content into a Content Server (normally Redis), from a directory tree or S3 bucket, or from FMI's Radon database. Also configuration generators. |
| `src/servers/` | `corbaContentServer`, `corbaDataServer`, `corbaQueryServer`, `corbaGridServer`, `httpContentServer`, `httpServer` | **Standalone servers**: run a grid-content service in its own process and expose it over CORBA or HTTP. |
| `src/clients/` | about 150 `cs_*`, `ds_*`, `qs_*` programs | **One Content, Data or Query Server method each**, for scripting, testing and debugging. |
| `src/files/` | `grid_dump`, `grid_info`, `grid_query`, `grid_images`, `grid_maps`, … | Open grid files directly with grid-files and print or render them. |
| `src/utils/` | `gu_*` | Helpers: Lua function execution, geometry and polygon queries, PNG merging, configuration inspection. |
| `src/python/` | `cs_*.py`, `graph_*.py`, `anim_*.py` | Python clients built on the grid-content omniORB Python bindings. |

`cfg/` holds example configurations for every server and ingest program,
`systemd/radon2smartmet.service` is the production unit for the Radon ingest.

## 2. Building and installing

```bash
make                  # every src/*/*.cpp -> bin/<dir>/<name>
make CORBA=disabled
make install          # -> $(bindir)/{clients,files,fmi,servers,utils}/, man pages
make rpm              # smartmet-tools-grid
```

* **A new `.cpp` in an existing directory is a new program.** The Makefile globs
  `src/*/*.cpp`. A new directory also needs its `mkdir` and copy lines in `install` and
  a line in the spec's `%files`.
* **The programs are not installed on `PATH`.** They go into subdirectories of
  `/usr/bin` (`/usr/bin/fmi/radon2smartmet`, `/usr/bin/clients/cs_getProducerInfoList`,
  …). Call them with the full path or add the directories to `PATH`.
* CORBA builds use the **installed** grid-content stub headers
  (`/usr/include/smartmet/grid-content/*/corba/stubs`).
* There are no tests. The programs are themselves the test tools for the libraries.

## 3. Environment

| Variable | Used by |
|----------|---------|
| `SMARTMET_ENV_FILE` | Every sample configuration starts with `@include "$(SMARTMET_ENV_FILE)"`. The file defines the Redis, PostgreSQL, CORBA and HTTP addresses and the directory variables (`cfg/smartmet-env.cfg`, `smartmet-dev-env.cfg` and `smartmet-test-env.cfg` are examples). Without it, the configurations fall back to `$(HOME)/workspace/smartmet/smartmet-tools-grid/cfg/smartmet-dev-env.cfg`. |
| `SMARTMET_GRID_CONFIG_FILE` | The grid-files configuration (`grid-files.conf`) for the programs that parse grid files directly (`grid_*`, some `cs_*`). |
| `SMARTMET_CS_IOR`, `SMARTMET_DS_IOR`, `SMARTMET_QS_IOR` | The default CORBA IORs of the Content, Data and Query Servers for the client programs. |
| `GRID_FILES_LIBRARY_CONFIG_FILE`, `GRID_TOOLS_CONFIG_DIR`, … | Referenced from the sample configurations; defined in the environment file. |

## 4. filesys2smartmet

```
filesys2smartmet <configFile> <loopWaitTime>
```

It scans directories (and, with the memory mapper enabled, S3 buckets) for grid files,
parses them, and keeps a Content Server in sync with what it finds. With
`loopWaitTime = 0`, it runs one pass and exits. Otherwise it repeats the pass every
`loopWaitTime` seconds.

### What one pass does

1. **Re-reads the `locations`** from the configuration file (so they can change while it
   runs), then sends an `UPDATE_LOOP_START` event to the target.
2. **Lists the files** in every location matching its `patterns`: `type = "FS"` through
   `DataFetcher_filesys`, `type = "S3"` through `DataFetcher_network` with the location's
   credentials.
3. **Producers.** It reads `producerDefFile` (`abbr;name;title;description`) and adds
   the producers that the target does not have yet.
4. **Generations are derived from file names.** After the optional Lua
   `filenameFixer` (a type-6 Lua function that may rewrite the name), the name is split
   on `_`: the first part is the producer **abbreviation** from `producerDefFile`, and
   the second is the analysis time. `ECMWF_20260925T000000_t.grib` belongs to producer
   `ECMWF`, generation `ECMWF:20260925T000000`. Files whose name does not have at least
   three `_`-separated parts, or whose abbreviation is unknown, are **ignored
   silently**. New generations are added with status **Ready** at once. Generations that
   no longer have files are deleted.
5. **Files.** For every target file with this program's `source-id` that is no longer
   on disk, the file (and its content) is deleted. A file whose modification time
   changed is deleted and added again. A new file is parsed with grid-files
   (`GridFile::read()`), one `ContentInfo` per message, and added with
   `addFileInfoWithContentList()`.
6. With a Redis target, `syncFilenames()` repairs the filename → id hash. Then it sends
   `UPDATE_LOOP_END`.

### Ownership and caching

* **`source-id`** marks every record the program writes. It only ever deletes records
  with its own source id, so several ingest programs (and radon2smartmet) can share
  one registry. Give each instance a different id.
* **Content cache.** Parsing thousands of GRIB files on every pass would be slow, so the
  parsed content of each file is written to `<cacheDir>/F2S_<hash of server:filename>`,
  headed by the file's modification time. On the next pass, a cache file with the same
  modification time is used instead of parsing. Cache files for files that have
  disappeared are removed at the start of each pass.
* **Incomplete identification.** If a message has no geometry id or no FMI parameter
  id (the grid-files configuration does not know it yet), the file is stored with its
  modification time minus one second. The next pass then sees a changed file and parses
  it again, so fixing the grid-files configuration eventually fixes the registry without
  a restart.

## 5. radon2smartmet

```
radon2smartmet <configFile> [loopWaitTime]
```

It copies the content metadata of FMI's Radon database (PostgreSQL) into the Content
Server. **It does not open any grid files.** Radon already stores every message's file
location, byte offset, length, parameter, level, times, forecast type and geometry,
and radon2smartmet turns those rows into `FileInfo` and `ContentInfo` records.
Production runs it from `systemd/radon2smartmet.service` (`ExecStart=... ${CONFIG} 300`,
`EnvironmentFile=/etc/smartmet/radon2smartmet.env`, restarted on failure). SIGINT,
SIGTERM and SIGHUP make it finish the current step and exit.

### What one pass does

1. `UPDATE_LOOP_START`, then it re-reads the **producer file** (`producerFile`, see
   below). Only producers listed there are synchronised.
2. **Producers, generations, geometries**: read from Radon (`fmi_producer` and the
   `ss_state_v` view) and from the target, then the differences are applied. A
   generation that disappears from Radon is deleted, and generations and files take their
   deletion times from Radon's `delete_time`.
3. **Files and content** (`updateTargetFiles()`): for each producer, it reads the
   forecast times, then the rows of each forecast time's table
   (`readSourceFilesByForecastTime()`). New files are sent with
   `addFileInfoListWithContent()` in batches of at most `maxMessageSize` files. Then
   `saveTargetContent()` adds the missing content records and `deleteTargetFiles()`
   deletes this source's files that were not seen in the pass. (`deleteOldFileRecords()`
   only prunes the program's own in-memory bookkeeping.)
4. **Generation status**: a generation becomes **Ready** only when Radon's
   `ss_forecast_status` says `READY` for it (`readReadyGenerations()`). With the
   synchronisation flag, a generation is set ready only when every producer on the same
   producer-file line has that generation ready.
5. `syncFilenames()` (Redis), then `UPDATE_LOOP_END`.

A failure inside a pass is printed, and the next pass starts after the wait time. A lost
database connection is re-established at the start of the next pass.

### The producer file

```
# producers;syncFlag;updateStart;updateInterval;[geometries];[acceptedParams];[ignoredParams];[cachedLevelTypes]
ECG,ECGMTA;1;2;1
ECGEPS,ECGEPSMTA;1;3;2
```

* **producers**: related producers, whose generations are kept in step when **syncFlag**
  is 1;
* **updateStart**: the pass number on which the producer is first updated (important
  producers first after a restart);
* **updateInterval**: update the producer only every N passes;
* then optional lists of accepted geometries, accepted parameters, ignored parameters,
  and level types whose files should be cached locally by the Data Server (sets
  `LocalCacheRecommended`).

## 6. Other FMI programs

* **`radon2config`** generates grid-files configuration CSVs from Radon's geometry,
  parameter and level definitions: `fmi_geometries.csv` (its header says "generated by
  radon2config"), `fmi_levels.csv`, `fmi_levelId_grib1.csv`, `fmi_levelId_grib2.csv` and
  others. Run it when Radon gets new geometries or parameters, and
  review the diff before installing the files.
* **`radon2csv <outputDir> <sourceId> <producerListFile> <dbConnectionString>`** exports
  Radon's producers, generations, files and content as CSV files that a Content Server
  can import (for example a `MemoryImplementation` / `file` content source).
* **`createLandSeaMap <coverDir> <width> <height> <mapFile>`** builds a land-sea mask
  file for `Map::topography`.

## 7. Standalone servers

Each server reads a libconfig file (`cfg/corba-*-server.cfg`, `cfg/http-content-server.cfg`),
builds the grid-content objects, and serves them:

| Server | Serves | Built from |
|--------|--------|------------|
| `corbaContentServer` | Content Server over CORBA | a Redis, PostgreSQL, CORBA, HTTP or memory source, optionally behind a `CacheImplementation` |
| `corbaDataServer` | Data Server over CORBA | `DataServer::ServiceImplementation` over a (remote) Content Server |
| `corbaQueryServer` | Query Server over CORBA | `QueryServer::ServiceImplementation` over remote Content and Data Servers |
| `corbaGridServer` | all three in **one process** over CORBA | a content source + cache + Data Server + Query Server; the in-process equivalent of the grid engine without smartmetd |
| `httpContentServer` | Content Server over the grid-content HTTP transport | a Redis, CORBA, HTTP or memory source, optionally cached |
| `httpServer` | static files: `httpServer <port> <rootDir>` (libmicrohttpd) | |

The CORBA servers print their IORs, or write them to the configured `iorFile`s
(`smartmet.tools.grid.content-server.iorFile`, …). Clients and the grid engine
(`remote = true` + `ior`) use those IORs. `smartmet.tools.grid.corba-server.address` /
`.port` fix the listening address, so that the IOR stays the same across restarts.

These servers matter when the Data Server has to run on the machines that mount the
grid files, or when one content cache is shared by many consumers. Most installations
run everything inside smartmetd through the grid engine instead.

## 8. Client programs

Each `cs_*`, `ds_*` and `qs_*` program calls **one** service method and prints the
result, for example:

```bash
cs_getProducerInfoList 0 -redis 127.0.0.1 6379 a. mypassword
cs_getGenerationInfoListByProducerName 0 ECG -http http://host/grid-admin
cs_getContentListByFileId 0 12345                       # CORBA, SMARTMET_CS_IOR
ds_getGridValueByPoint 0 12345 3 1 24.9 60.2 1          # CORBA, SMARTMET_DS_IOR
```

The first argument is always the session id (0 is fine; sessions are not enforced). The
backend is chosen by the **trailing** arguments:

| Trailing arguments | Backend | Available in |
|--------------------|---------|--------------|
| `-redis <address> <port> <tablePrefix> <password>` | `RedisImplementation`, direct | `cs_*` |
| `-pg <connectionString>` | `PostgresqlImplementation`, direct | `cs_*` |
| `-http <url>` | `HTTP::ClientImplementation` (a grid-admin URL or `httpContentServer`) | `cs_*` |
| `-ior <ior>` | CORBA client | all |
| none | CORBA client with `SMARTMET_CS_IOR` / `SMARTMET_DS_IOR` / `SMARTMET_QS_IOR` | all |

The `ds_*` and `qs_*` programs only speak CORBA. Every program prints the call's
duration, and `ERROR (<code>) : <text>` with a non-zero exit status on failure. Writing
programs (`cs_add…`, `cs_delete…`, `cs_set…`, `cs_clear`) change the registry directly.
Point them at a scratch table prefix when experimenting.

## 9. File and utility programs

* **`grid_dump <file> [-coordinates] [-data] [-bitmap]`** prints every message's
  sections and identification as grid-files sees them. Use it first when a file is
  identified wrongly: it shows which GRIB fields the identification matched. `grid_info`
  gives a shorter summary, and `grid_query` reads values.
* **`grid_images`, `grid_maps`, `grid_submaps`, `grid_getIsobandImage`,
  `grid_getIsolineImage`** render messages to images without a server.
* **`grid_add`, `grid_create`** write GRIB files.
* **`gu_executeLuaFunction`, `gu_getLuaFunctions`** load a Lua function file the way the
  Query Server does, and list or run its functions. Use them to test a new Lua function
  without a server.
* **`gu_getGridGeometryIdListByLatLon`, `gu_getGridPointsInsidePolygon`, …** answer
  geometry questions from the grid-files configuration.
* **`gu_showConfigurationAttributes`, `gu_replaceConfigurationAttributes`** show a
  libconfig file after `@include` and variable expansion, or write a copy with
  attributes replaced.

These programs call `Identification::gridDef.init()` with `SMARTMET_GRID_CONFIG_FILE`,
so they identify parameters and geometries exactly as a server with the same
configuration would.

## 10. Recipes

### 10.1 A local registry for development or tests

```bash
redis-server --port 6380 --dir /tmp/gridredis &
# filesys-to-smartmet.cfg: storage type redis, port 6380, a table prefix,
# one FS location pointing at the grid files, producerDefFile listing their abbreviations
filesys2smartmet my-filesys-to-smartmet.cfg 0     # one pass, then exit
cs_getContentCount 0 -redis 127.0.0.1 6380 a. ""  # check
redis-cli -p 6380 SAVE                             # writes /tmp/gridredis/dump.rdb
```

The grid engine's test package (`smartmet-engine-grid-test`) ships such a dump
(`testdata/grid/redis/redis-server.rdb`) for the GRIB files in `smartmet-test-data`.
Its history does not record how it was produced; the procedure above is the natural way
to regenerate it. The file names in the registry must then match the paths the tests use.

### 10.2 Why is a parameter missing?

1. `grid_dump` the file: are the FMI parameter id, level id and geometry id set?
2. If not, fix the grid-files mapping CSVs (see the grid-files docs). filesys2smartmet
   re-reads such files automatically ([§4](#4-filesys2smartmet)); with radon2smartmet,
   the ids come from Radon.
3. `cs_getContentListByFileName 0 <file> -redis …`: is the content in the registry?
4. The grid-admin browser (or its API with `source=engine`) shows whether the engine's
   cache has it too.
5. If the content is there but queries still miss it, check the Query Server's parameter
   mappings and producer file (see the grid-content and grid-engine guides).

## 11. Common tasks

### 11.1 Adding a client program

Copy the closest existing program. Keep the same argument order (session id first,
backend flags last), the same backend selection block, the same timing output, and the
same `ERROR (<code>)` line and exit codes. Add a man page to `doc/man/` if the program
is meant for users.

### 11.2 Changing an ingest program

* Keep **source-id ownership**: only ever delete records with your own `mSourceId`.
* Use the **batch** methods (`addFileInfoListWithContent`, `deleteFileInfoListByFileIdList`,
  `deleteGenerationInfoListByIdList`). Every single-record call is a Redis round trip
  and an event that every cache and Data Server has to process.
* Send `UPDATE_LOOP_START` / `UPDATE_LOOP_END` around each pass. The content cache
  records content hashes at these events, and they mark the passes in consumers' logs.
* Test against a scratch Redis prefix and watch the event list
  (`cs_getEventInfoList`) to see what your change emits.

## 12. Known pitfalls

* **filesys2smartmet registers every new file twice.** In `readSourceContent()`, the
  "missing geometry or parameter id" check runs **before** `setMessageContent()` fills in
  those ids, so it is always true on a fresh parse. Every newly found file is therefore
  stored with the "minus one second" modification time
  ([§4](#4-filesys2smartmet)). On the next pass, it is deleted and added again (this
  time from the content cache, with the correct time). Each new file thus generates a
  `FILE_ADDED`, a `FILE_DELETED` and another `FILE_ADDED` event. It also disappears
  briefly from the registry one pass after it appeared.
* **File names decide producer and generation** in filesys2smartmet. A file that does
  not follow `ABBR_ANALYSISTIME_…` (after the Lua fixer) is skipped without any message.
* **S3 locations need the memory mapper.** filesys2smartmet only lists `type = "S3"`
  locations when `grid-files.memoryMapper.enabled` is true.
* **Generations are Ready immediately** in filesys2smartmet, even while the model run is
  still being written. Use a staging directory and move complete runs in.
* **`/usr/bin` subdirectories.** The installed programs are not on `PATH`.
* **Destructive clients run without confirmation.** `cs_clear` empties the whole
  registry that the arguments point at.
