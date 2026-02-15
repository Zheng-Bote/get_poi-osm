/**
 * SPDX-FileComment: Main entry point for the get_poi-osm application
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file main.cpp
 * @brief  Implements the main entry point and CLI argument parsing using CLI11.
 * @version 0.1.0
 * @date 2026-02-15
 *
 * @author ZHENG Robert
 * @license MIT License
 */

#include <CLI/CLI.hpp>
#include <curl/curl.h>
#include <print>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include "PoiOsm.hpp"

namespace {

// Helper to split string by delimiter
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

} // namespace

int main(int argc, char** argv) {
    // Initialize libcurl globally
    curl_global_init(CURL_GLOBAL_ALL);

    CLI::App app{"OSM POI finder (JSON, 100km radius)"};
    app.set_version_flag("--version", "1.0");

    double lat = 0.0;
    double lon = 0.0;
    std::string address;
    std::vector<std::string> rawWhitelist;
    int radius = 100000;

    auto latOpt = app.add_option("-l,--lat", lat, "Latitude");
    auto lonOpt = app.add_option("-L,--lon", lon, "Longitude");
    auto addrOpt = app.add_option("-a,--address", address, "Address");
    app.add_option("-w,--whitelist", rawWhitelist, "Whitelist entry key[=value], e.g. amenity=restaurant");
    app.add_option("-r,--radius", radius, "Search radius in meters")->default_val(100000);

    // Custom validation: Either (lat AND lon) OR address must be set
    latOpt->needs(lonOpt);
    lonOpt->needs(latOpt);
    
    CLI11_PARSE(app, argc, argv);

    bool hasLatLon = !latOpt->empty() && !lonOpt->empty();
    bool hasAddr = !addrOpt->empty();

    if (!hasLatLon && !hasAddr) {
        std::println(stderr, "Provide either --lat/--lon or --address");
        return 1;
    }

    // Parse whitelist
    std::vector<PoiWhitelistEntry> whitelist;
    for (const auto& entry : rawWhitelist) {
        auto parts = split(entry, '=');
        PoiWhitelistEntry w;
        if (!parts.empty()) {
            w.key = parts[0];
            // Trim whitespace (simple version)
            w.key.erase(0, w.key.find_first_not_of(" \t"));
            w.key.erase(w.key.find_last_not_of(" \t") + 1);
            
            if (parts.size() > 1) {
                w.value = parts[1];
                w.value.erase(0, w.value.find_first_not_of(" \t"));
                w.value.erase(w.value.find_last_not_of(" \t") + 1);
            }
            if (!w.key.empty()) {
                whitelist.push_back(w);
            }
        }
    }

    PoiOsmClient client;
    std::expected<nlohmann::json, std::string> result;

    if (hasLatLon) {
        result = client.queryByCoordinates(lat, lon, radius, whitelist);
    } else {
        result = client.queryByAddress(address, radius, whitelist);
    }

    if (result) {
        std::println("{}", result->dump(4)); // Pretty print
    } else {
        nlohmann::json err;
        err["schema_version"] = 1;
        err["error"] = result.error();
        std::println(stderr, "{}", err.dump(4));
        curl_global_cleanup();
        return 1;
    }

    curl_global_cleanup();
    return 0;
}
