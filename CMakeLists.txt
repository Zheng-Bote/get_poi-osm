cmake_minimum_required(VERSION 3.28)

project(get_poi-osm
    VERSION 1.0.0
    DESCRIPTION "Get POIs from OpenStreetMap"
    HOMEPAGE_URL "https://github.com/Zheng-Bote/get_poi-osm"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# nlohmann_json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# CLI11
FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.4.1
)
FetchContent_MakeAvailable(CLI11)

# libcurl
find_package(CURL REQUIRED)

add_library(get_poi-osm SHARED
    src/PoiOsm.cpp
    include/PoiOsm.hpp
)

target_link_libraries(get_poi-osm
    PUBLIC
        nlohmann_json::nlohmann_json
        CURL::libcurl
)

target_include_directories(get_poi-osm
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_executable(get_poi-osm-cli
    src/main.cpp
)

target_link_libraries(get_poi-osm-cli
    PRIVATE
        get_poi-osm
        CLI11::CLI11
)

include(GNUInstallDirs)

install(TARGETS get_poi-osm get_poi-osm-cli
    EXPORT get_poi-osmTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

install(FILES
    schema/get_poi-osm-schema-v1.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/get_poi-osm
)

