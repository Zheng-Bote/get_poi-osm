# get_poi-osm

A command‑line tool and shared library for querying OpenStreetMap POIs around a coordinate or address. \
Supports whitelisting, language‑aware POI names, JSON output, and CMake FetchContent integration.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)]()
[![CMake](https://img.shields.io/badge/CMake-3.28+-blue.svg)]()

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/Zheng-Bote/get_poi-osm?logo=GitHub)](https://github.com/Zheng-Bote/get_poi-osm/releases)

[Report Issue](https://github.com/Zheng-Bote/get_poi-osm/issues) · [Request Feature](https://github.com/Zheng-Bote/get_poi-osm/pulls)

---

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
<!-- END doctoc generated TOC please keep comment here to allow auto update -->

---

## Description

A lightweight C++23 command‑line tool and shared library for querying OpenStreetMap POIs (Points of Interest) around a coordinate or address.
Supports whitelisting, language‑aware POI names, JSON output, and CMake FetchContent integration.

## Features

- Query POIs within a configurable radius (default: 100 km)
- Input via latitude/longitude or free‑form address (Nominatim geocoding)
- Overpass API backend
- Optional whitelist filters (e.g. tourism, tourism=viewpoint, amenity=restaurant)
- English‑preferred POI names (name:en fallback)
- JSON output following a versioned schema (schema_version = 1)
- Installable shared library + CLI tool
- FetchContent‑friendly CMake package

> [!NOTE]
> There is also a Qt6 version of this project: [qt_get_poi-osm](https://github.com/Zheng-Bote/qt_get_poi-osm).

## Library Shortcut / Developer Introduction

**get_poi-osm** is a lightweight C++23 library designed to make OpenStreetMap POI discovery effortless.

It wraps Nominatim (geocoding) and Overpass (POI queries) behind a clean, synchronous interface using `std::expected` for error handling and returns structured, versioned JSON results via `nlohmann/json`.

If you need to fetch points of interest around a coordinate or address — restaurants, viewpoints, theme parks, museums, shops, or any other OSM‑tagged feature — this library gives you a simple, modern API that works out of the box.

### What the library gives you

- A single class: `PoiOsmClient`
- Modern C++ error handling: `std::expected<nlohmann::json, std::string>`
- Input:
  - latitude/longitude
  - or free‑form address (auto‑geocoded)
- Optional **whitelist filters**:
  - tourism
  - tourism=viewpoint
  - amenity=restaurant
  - any OSM key or key=value pair
- Output:
  - A stable, versioned JSON object (schema_version = 1)
  - Includes query metadata, resolved coordinates, and all matching POIs
  - English‑preferred POI names (name:en fallback)

### Why it’s easy to integrate

- Distributed as an installable shared library
- Fully FetchContent‑compatible
- Modern C++23 codebase
- Uses industry-standard `nlohmann/json` and `libcurl`
- Clean header interface (PoiOsm.hpp)

### Typical usage pattern

```cpp
#include <PoiOsm.hpp>
#include <print>

int main() {
    PoiOsmClient client;

    auto result = client.queryByCoordinates(48.13743, 11.57549, 1000,
                                            {{"tourism", "viewpoint"}});

    if (result) {
        std::println("{}", result->dump(4));
    } else {
        std::println(stderr, "Error: {}", result.error());
    }
    return 0;
}
```

This example fetches all viewpoints within 1 km of Munich.

### When to use this library

- You’re building a CLI tool, desktop app, or service that needs OSM POIs
- You want a simple C++ API instead of manually crafting Overpass queries
- You need structured JSON output for downstream processing
- You want a stable, versioned schema for long‑term compatibility

## Pre‑Requisites

- C++23 compiler
- GCC ≥ 12
- Clang ≥ 15
- MSVC ≥ 19.36
- libcurl
- CMake ≥ 3.28
- Internet access (Nominatim + Overpass API)

## Dependencies

**Runtime**

- OpenStreetMap Nominatim (geocoding)
- Overpass API (POI queries)
- libcurl

**Build‑time**

- nlohmann_json (fetched via CMake)
- CLI11 (fetched via CMake)
- libcurl
- CMake ≥ 3.28

## Build

```bash
cmake -B build -S .
cmake --build build -j$(nproc)
```

This builds:

- Shared library: libget_poi-osm.so (Linux)
- CLI tool: get_poi-osm-cli

## Install

```bash
sudo cmake --install build
```

Installs:

```bash
/usr/local/bin/get_poi-osm-cli
/usr/local/lib/libget_poi-osm.so
/usr/local/include/PoiOsm.hpp
/usr/local/share/get_poi-osm/poi-osm-schema-v1.json
/usr/local/lib/cmake/get_poi-osm/
```

## Usage (CLI)

### Query by coordinates

```bash
get_poi-osm-cli --lat 48.13743 --lon 11.57549
```

### Query by address

```bash
get_poi-osm-cli --address "Marienplatz, Munich"
```

### Whitelist examples

#### All tourism POIs

```bash
get_poi-osm-cli --lat 48.13743 --lon 11.57549 --whitelist tourism
```

#### Only viewpoints

```bash
get_poi-osm-cli --lat 48.13743 --lon 11.57549 --whitelist tourism=viewpoint
```

#### Viewpoints OR theme parks

```bash
get_poi-osm-cli \
  --lat 48.13743 --lon 11.57549 \
  --whitelist tourism=viewpoint \
  --whitelist tourism=theme_park
```

#### Restaurants only

```bash
get_poi-osm-cli --lat 48.13743 --lon 11.57549 --whitelist amenity=restaurant
```

#### Tourism + Restaurants

```bash
get_poi-osm-cli \
  --lat 48.13743 --lon 11.57549 \
  --whitelist tourism \
  --whitelist amenity=restaurant
```

## JSON Output Schema

The tool outputs a versioned JSON structure (schema_version = 1).
The full schema is installed under:

```bash
/usr/local/share/get_poi-osm/poi-osm-schema-v1.json
```

## Using the Library via CMake FetchContent

### 1. Add FetchContent to your project

```cmake
include(FetchContent)

FetchContent_Declare(
    get_poi_osm
    GIT_REPOSITORY https://github.com/Zheng-Bote/get_poi-osm.git
    GIT_TAG main
)

FetchContent_MakeAvailable(get_poi_osm)
```

### 2. Link against the library

```cmake
add_executable(mytool main.cpp)
target_link_libraries(mytool PRIVATE get_poi-osm::get_poi-osm)
```

## Example Program Using the Library

```cpp
#include <PoiOsm.hpp>
#include <print>
#include <vector>

int main()
{
    PoiOsmClient client;

    std::vector<PoiWhitelistEntry> whitelist;
    whitelist.push_back({"tourism", "viewpoint"});

    // query by coordinates, where radius is in meters
    auto result = client.queryByCoordinates(48.13743, 11.57549, 100000, whitelist);

    if (result) {
        std::println("{}", result->dump(4));
    } else {
        std::println(stderr, "Error: {}", result.error());
        return 1;
    }

    return 0;
}
```

## Build with CMake

```cmake
cmake_minimum_required(VERSION 3.28)
project(mytool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    get_poi_osm
    GIT_REPOSITORY https://github.com/Zheng-Bote/get_poi-osm.git
    GIT_TAG main
)

FetchContent_MakeAvailable(get_poi_osm)

find_package(CURL REQUIRED)

add_executable(mytool main.cpp)
target_link_libraries(mytool PRIVATE get_poi-osm::get_poi-osm)
```

---

## License

Distributed under the MIT License. See LICENSE for more information.

Copyright (c) 2026 ZHENG Robert

## Author

[![Zheng Robert - Core Development](https://img.shields.io/badge/Github-Zheng_Robert-black?logo=github)](https://www.github.com/Zheng-Bote)

### Code Contributors

![Contributors](https://img.shields.io/github/contributors/Zheng-Bote/get_poi-osm?color=dark-green)

---

**Happy coding! 🚀** :vulcan_salute:
