# smartmet-tools-grid — Feature List

A structured inventory of capabilities provided by the smartmet-tools-grid
package. Use as a checklist when drafting release notes. When new
functionality is added, append the new entry under the matching section
(and bump the *Last updated* line at the bottom).

`smartmet-tools-grid` is a collection of **command-line programs** for
the SmartMet Server grid support system. Unlike the other repositories
in the ecosystem, it does not produce a library — every `.cpp` under
`src/` compiles to one standalone binary, installed under
`$(prefix)/bin/{clients,files,fmi,servers,utils}/`. Auxiliary Python
scripts ship under `src/python/`.

---

## 1. Standalone server daemons (`src/servers/`)

Each binary wraps one grid-content service behind a network protocol so
it can run as a separate process from the SmartMet Server.

- **`corbaContentServer`** — Content Server over CORBA (omniORB).
- **`corbaDataServer`** — Data Server over CORBA.
- **`corbaQueryServer`** — Query Server over CORBA.
- **`corbaGridServer`** — combined Content + Data + Query servers in a
  single CORBA process.
- **`httpContentServer`** — Content Server over JSON-over-HTTP.
- **`httpServer`** — generic HTTP server for grid-content APIs.

Each daemon has a matching example config under `cfg/` (e.g.
`corba-content-server.cfg`, `http-content-server.cfg`,
`corba-content-server-memory.cfg`) and a man page under `doc/man/`.

## 2. Content Server CLI clients (`cs_*`, 116 programs)

One binary per Content Server service method. Every program accepts the
same backend-selection trailing flags (`-http`, `-redis`, `-pg`, `-ior`,
or defaults to `SMARTMET_CS_IOR`).

### Catalogue management
- **Producers** — `cs_addProducerInfo`, `cs_deleteProducerInfoById`,
  `cs_deleteProducerInfoByName`, `cs_deleteProducerInfoListBySourceId`,
  `cs_getProducerInfoById`, `cs_getProducerInfoByName`,
  `cs_getProducerInfoList`, plus list-by-source / by-parameter /
  by-name+geometry / counts.
- **Generations** — `cs_addGenerationInfo`, deletes by ID / name /
  producer / source, listings by producer / geometry / source,
  status setters and getters, last-generation lookups.
- **Geometries** — `cs_addGeometryInfo`, deletes by ID / generation /
  producer / source, listings by generation / producer / source.
- **Files** — `cs_addFileInfo`, `cs_addFileInfoListFromFile`,
  `cs_addFileInfoWithContentList`, `cs_addFileAndContentListFromFile`,
  deletes by ID / name / list / generation / producer / source,
  listings and counts at each scope.
- **Content records** — `cs_addContentInfo`, deletes by ID / file /
  generation / producer / source, listings by file ID / file name /
  generation / producer / parameter / parameter+generation /
  parameter+producer / parameter+generation+time, time/level/parameter-
  key listings, geometry-id listings, integrity check
  (`cs_getContentListOfInvalidIntegrity`).
- **Events** — `cs_getLastEventInfo`, `cs_getEventInfoList`,
  `cs_getEventInfoCount`.
- **Service / admin** — `cs_clear`, `cs_reload`, `cs_getContentCount`,
  `cs_getCsvFiles`.

### Counts and aggregates
- File counts by generation / producer / source and bulk
  `cs_getFileInfoCountsByGenerations`,
  `cs_getFileInfoCountsByProducers`.

## 3. Data Server CLI clients (`ds_*`, 31 programs)

One binary per Data Server service method.

- **Whole-grid retrieval** — `ds_getGridData`,
  `ds_getGridValueVectorByGeometry`,
  `ds_getGridLatlonCoordinatesByGeometry`, `ds_getGridCoordinates`,
  `ds_getGridAttributeList`, `ds_getGridProperties`,
  `ds_getGridMessageBytes`.
- **Point lookups** — `ds_getGridValueByPoint`,
  `ds_getGridValueListByPointList`, `ds_getGridValueListByCircle`,
  `ds_getGridValueListByRectangle`, `ds_getGridValueListByPolygon`,
  `ds_getGridValueListByPolygonPath`.
- **Level interpolation** — same point lookups with
  `…ByLevelAnd…` variants.
- **Time interpolation** — same point lookups with `…ByTimeAnd…`
  variants.
- **Combined time + level** — `ds_getGridValueByTimeLevelAndPoint`.
- **Property-by-coordinate** — `ds_getPropertyValuesByCoordinates`.
- **Counts** — `ds_getGridFileCount`.

## 4. Query Server CLI clients (`qs_*`, 6 programs)

High-level query execution and parameter sampling:

- **`qs_executeQuery`** — run a full `QueryServer::Query`.
- **`qs_getParameterValueByPointAndTime`**
- **`qs_getParameterValuesByPointListAndTime`**
- **`qs_getParameterValuesByPointAndTimeList`**
- **`qs_getParameterVerticalValueVectorByPointAndTime`**
- **`qs_getParameterValueVectorByGeometryAndTime`**

## 5. Grid file inspection tools (`grid_*`, `src/files/`)

Stand-alone tools that open and inspect single grid files via
`smartmet-library-grid-files`:

- **`grid_info`** — print message metadata (producer, parameter, geometry,
  forecast time, …).
- **`grid_dump`** — dump message values.
- **`grid_create`** / **`grid_add`** — write and append messages.
- **`grid_query`** — point/area queries against a grid file.
- **`grid_images`** — render messages to PNG (color-mapped images).
- **`grid_maps`** / **`grid_submaps`** — render with map overlay
  (full / partial geographic windows).
- **`grid_getIsobandImage`** / **`grid_getIsolineImage`** — render
  contour outputs.

## 6. FMI ingestion tools (`src/fmi/`)

Data-ingestion pipelines that populate the Content Information Storage
from FMI's production systems:

- **`radon2smartmet`** — sync grid-file metadata from the Radon database
  into a Content Server (Redis/PostgreSQL/CORBA/HTTP). Includes a
  systemd service unit (`systemd/radon2smartmet.service`).
- **`radon2config`** — generate a Content Server configuration from a
  Radon database.
- **`radon2csv`** — export Radon metadata to CSV.
- **`filesys2smartmet`** — watch a filesystem tree and register newly
  appearing grid files with a Content Server. Configurable via
  `cfg/filesys-to-smartmet.cfg` and `cfg/filenameFixer.lua`. Detailed
  user guide in `doc/filesys2smartmet.md` / `.pdf`.
- **`createLandSeaMap`** — build the land/sea mask file used by the
  topography module of `smartmet-library-grid-files`.

## 7. Grid utilities (`gu_*`, `src/utils/`)

Smaller helper tools for development and operations:

- **`gu_executeLuaFunction`** — invoke a configured Lua function against
  test inputs from the CLI.
- **`gu_getLuaFunctions`** — enumerate the Lua functions loaded from a
  configured directory.
- **`gu_getGridLatLonCoordinatesByGeometryId`** — print latlon vectors
  for a geometry.
- **`gu_getGridGeometryIdListByLatLon`** — list geometries that cover a
  point.
- **`gu_getGridPointsInsideCircle`** /
  **`gu_getGridPointsInsidePolygon`** /
  **`gu_getGridPointsInsidePolygonPath`** — enumerate grid cells inside
  a shape.
- **`gu_getEnlargedPolygon`** — buffer a polygon by N grid cells.
- **`gu_getLatLonDistance`** — great-circle distance between two
  lat/lon points.
- **`gu_mergePngFiles`** / **`gu_mergePngFilesSeq`** — composite
  multiple PNG layers (single-shot and frame-sequence variants).
- **`gu_newbase2fmi`** — translate a Newbase parameter ID to its FMI
  identifier.
- **`gu_showConfigurationAttributes`** /
  **`gu_replaceConfigurationAttributes`** — inspect or rewrite the
  attribute tree of a libconfig file.

## 8. Python client scripts (`src/python/`)

Reference implementations and utilities that use the Content Server's
Python bindings:

- **Content Server queries** — `cs_getProducerInfoById`,
  `cs_getProducerInfoByName`, `cs_getProducerInfoList`,
  `cs_getGenerationInfoById`, `cs_getGenerationInfoListByProducerId`,
  `cs_getGenerationInfoListByProducerName`,
  `cs_getContentListByGenerationId`, `cs_getContentListByProducerId`.
- **Graphing helpers** — `graph_getArrows`,
  `graph_getArrowsByGeometryId`, `graph_getCoverBorders`,
  `graph_getCoverBordersByGeometryId`, `graph_getCoverFill`.
- **Animation builders** — `anim_getLatestAnimationByProducer`,
  `anim_getLatestAnimationByProducerAndGeometryId`.

## 9. Backend selection (clients only)

Every C++ client program accepts the same trailing flags:

- **`-ior <IOR>`** — talk to a CORBA Content Server. Default when no
  flag is given is to read `SMARTMET_CS_IOR` from the environment.
- **`-http <url>`** — talk over the Content Server JSON-over-HTTP API.
- **`-redis <address> <port> <tablePrefix>`** — go straight to Redis
  (Content Server clients only).
- **`-pg <connectionString>`** — go straight to PostgreSQL (Content
  Server clients only).

## 10. Configuration

Example configs under `cfg/`:

- **Server configs** — `corba-{content,data,query,grid}-server.cfg`,
  `http-content-server.cfg`, `corba-content-server-memory.cfg`.
- **Ingestion configs** — `filesys-to-smartmet.cfg`,
  `radon-to-smartmet.cfg`, `generate-fmig-files.cfg`,
  `filenameFixer.lua`.
- **Environment manifests** — `smartmet-env.cfg`,
  `smartmet-dev-env.cfg`, `smartmet-test-env.cfg` — define Redis /
  CORBA / HTTP / PostgreSQL endpoints; included by libconfig configs
  via `@include "$(SMARTMET_ENV_FILE)"`.
- **Catalogue / parameter** — `producers.cfg`, `producerDef.csv`,
  `parameters_mfig.cfg`, `preload.csv`.
- **API reference** — `contentServer_methods.html` (list of all
  Content Server methods callable from clients).

Required environment variables:
- `SMARTMET_GRID_CONFIG_FILE` — needed by all `grid_*` file tools.
- `SMARTMET_CS_IOR` — default CORBA IOR for clients.
- `SMARTMET_ENV_FILE` — global env manifest used by `filesys2smartmet`
  and `radon2smartmet`.

## 11. Documentation

- **`doc/grid-support.{md,html,pdf,odt,docx}`** — full grid support
  overview.
- **`doc/filesys2smartmet.{md,pdf,odt,docx}`** — user guide for the
  filesystem-watcher ingestion tool.
- **`doc/grid-support-img/`** — diagrams used in the documentation.
- **Man pages** (`doc/man/*.1`) for the public programs:
  `corbaContentServer`, `corbaDataServer`, `corbaGridServer`,
  `corbaQueryServer`, `httpContentServer`, `httpServer`,
  `filesys2smartmet`, `radon2smartmet`, `radon2config`, `radon2csv`,
  `createLandSeaMap`, `smartmet-cs`, `smartmet-ds`, `smartmet-qs`,
  `smartmet-gu`.

## 12. Build & integration

- **Output**: one binary per `.cpp` under `src/`, installed under
  `$(prefix)/bin/{clients,files,fmi,servers,utils}/`.
- **Build**: `make` (release) / `make debug`.
- **Skip CORBA**: `make CORBA=disabled`.
- **Install**: `make install`.
- **RPM**: `make rpm` (Spec file: `smartmet-plugin-grid-admin.spec`-style
  layout).
- **CI**: CircleCI on RHEL 8 / RHEL 10 (`fmidev/smartmet-cibase-{8,10}`
  Docker images). Optional CI knobs: `.circleci/disable-tests-in-ci`,
  `.circleci/enable-staging`.
- **Systemd**: `systemd/radon2smartmet.service` for the Radon-ingestion
  daemon.
- **Libraries used**: `smartmet-library-grid-files`,
  `smartmet-library-grid-content`, `smartmet-library-spine`,
  `smartmet-library-macgyver`, `smartmet-library-newbase`,
  `smartmet-library-gis`.
- **External libraries**: `libpq`, `hiredis`, `omniORB4`,
  `libmicrohttpd`, `libcurl`, `freetype2`, `libjpeg`, `libpng`.

---

*Last updated: 2026-06-01.*
