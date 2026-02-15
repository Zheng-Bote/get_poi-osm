<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Changelog](#changelog)
  - [[1.0.0] - 2026-02-15](#100---2026-02-15)
    - [Added](#added)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-02-15

### Added

- Initial release of `get_poi-osm`.
- C++23 shared library and CLI tool.
- Support for querying POIs by latitude/longitude and address (Nominatim).
- Overpass API integration for fetching POI data.
- Whitelist filtering support (e.g., `tourism=viewpoint`).
- JSON output format (Schema Version 1).
- CMake integration with `FetchContent` support.
