/**
 * SPDX-FileComment: Implementation of OpenStreetMap POI client
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file PoiOsm.cpp
 * @brief  Implements the PoiOsmClient class using libcurl and nlohmann::json.
 * @version 0.1.0
 * @date 2026-02-15
 *
 * @author ZHENG Robert
 * @license MIT License
 */

#include "PoiOsm.hpp"

#include <curl/curl.h>
#include <format>
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>

namespace {

// Helper for writing curl response to string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Simple RAII wrapper for CURL handle
struct CurlHandle {
    CURL* curl;
    CurlHandle() : curl(curl_easy_init()) {}
    ~CurlHandle() { if (curl) curl_easy_cleanup(curl); }
    operator CURL*() const { return curl; }
};

// URL Encoder helper
std::string urlEncode(CURL* curl, const std::string& value) {
    char* output = curl_easy_escape(curl, value.c_str(), static_cast<int>(value.length()));
    if (output) {
        std::string result(output);
        curl_free(output);
        return result;
    }
    return "";
}

// Perform HTTP GET or POST
std::expected<std::string, std::string> performRequest(const std::string& url, const std::string& postData = "") {
    CurlHandle handle;
    if (!handle.curl) return std::unexpected("Failed to initialize CURL");

    std::string readBuffer;
    curl_easy_setopt(handle.curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle.curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(handle.curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(handle.curl, CURLOPT_USERAGENT, "get_poi-osm/1.0 (https://github.com/Zheng-Bote/get_poi-osm)");
    curl_easy_setopt(handle.curl, CURLOPT_REFERER, "https://github.com/Zheng-Bote/get_poi-osm");
    
    // Follow redirects
    curl_easy_setopt(handle.curl, CURLOPT_FOLLOWLOCATION, 1L);

    if (!postData.empty()) {
        curl_easy_setopt(handle.curl, CURLOPT_POSTFIELDS, postData.c_str());
    }

    CURLcode res = curl_easy_perform(handle.curl);
    if (res != CURLE_OK) {
        return std::unexpected(std::format("CURL request failed: {}", curl_easy_strerror(res)));
    }

    long response_code;
    curl_easy_getinfo(handle.curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code >= 400) {
        return std::unexpected(std::format("HTTP Error: {}", response_code));
    }

    return readBuffer;
}

// Get current ISO8601 time
std::string currentIsoTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::gmtime(&now_c);
    char buf[30];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &now_tm);
    return std::string(buf);
}

} // namespace

std::expected<nlohmann::json, std::string> PoiOsmClient::queryByAddress(
    const std::string& address, int radiusMeters,
    const std::vector<PoiWhitelistEntry>& whitelist) {
    
    auto coords = geocodeAddress_(address);
    if (!coords) return std::unexpected(coords.error());

    nlohmann::json input;
    input["address"] = address;
    input["lat"] = nullptr;
    input["lon"] = nullptr;

    return queryOverpass_(coords->first, coords->second, radiusMeters, whitelist, input);
}

std::expected<nlohmann::json, std::string> PoiOsmClient::queryByCoordinates(
    double lat, double lon, int radiusMeters,
    const std::vector<PoiWhitelistEntry>& whitelist) {
    
    nlohmann::json input;
    input["address"] = nullptr;
    input["lat"] = lat;
    input["lon"] = lon;

    return queryOverpass_(lat, lon, radiusMeters, whitelist, input);
}

std::expected<std::pair<double, double>, std::string> PoiOsmClient::geocodeAddress_(const std::string& address) {
    CurlHandle handle; // Just for escaping
    std::string encodedAddr = urlEncode(handle.curl, address);
    std::string url = std::format("https://nominatim.openstreetmap.org/search?q={}&format=json&limit=1", encodedAddr);

    auto response = performRequest(url);
    if (!response) return std::unexpected(response.error());

    try {
        auto json = nlohmann::json::parse(*response);
        if (!json.is_array()) return std::unexpected("Invalid geocoding response: Not an array");
        if (json.empty()) return std::unexpected("No geocoding result for address");

        const auto& obj = json[0];
        // Nominatim returns lat/lon as strings
        double lat = std::stod(obj.value("lat", "0.0"));
        double lon = std::stod(obj.value("lon", "0.0"));

        if (lat == 0.0 && lon == 0.0) return std::unexpected("Invalid coordinates in geocoding response");

        return std::make_pair(lat, lon);
    } catch (const nlohmann::json::parse_error& e) {
        return std::unexpected(std::format("JSON parse error: {}", e.what()));
    } catch (const std::exception& e) {
        return std::unexpected(std::format("Error parsing coordinates: {}", e.what()));
    }
}

std::expected<nlohmann::json, std::string> PoiOsmClient::queryOverpass_(
    double lat, double lon, int radiusMeters,
    const std::vector<PoiWhitelistEntry>& whitelist,
    const nlohmann::json& queryInput) {

    std::string query = buildOverpassQuery_(lat, lon, radiusMeters, whitelist);
    
    // Overpass expects body: data=query
    CurlHandle handle;
    std::string postData = "data=" + urlEncode(handle.curl, query);

    auto response = performRequest("https://overpass-api.de/api/interpreter", postData);
    if (!response) return std::unexpected(response.error());

    try {
        auto json = nlohmann::json::parse(*response);
        if (json.contains("remark") && !json.contains("elements")) {
             return std::unexpected(std::format("Overpass API Error: {}", json["remark"].get<std::string>()));
        }

        if (!json.is_object() || !json.contains("elements")) {
             // Sometimes Overpass returns HTML on error
             if (response->find("<html") != std::string::npos) {
                 return std::unexpected("Overpass API returned HTML error (server might be busy)");
             }
             return std::unexpected("Invalid Overpass JSON response");
        }

        return buildResultJson_(lat, lon, radiusMeters, whitelist, json["elements"], queryInput);

    } catch (const nlohmann::json::parse_error& e) {
        return std::unexpected(std::format("JSON parse error: {}", e.what()));
    }
}

std::string PoiOsmClient::buildOverpassQuery_(
    double lat, double lon, int radiusMeters,
    const std::vector<PoiWhitelistEntry>& whitelist) const {
    
    std::string query = "[out:json][timeout:25];(";
    
    // Format lat/lon with high precision
    std::string center = std::format("around:{},{:.6f},{:.6f}", radiusMeters, lat, lon);

    if (whitelist.empty()) {
        query += std::format("node({});", center);
    } else {
        for (const auto& w : whitelist) {
            if (w.value.empty()) {
                query += std::format("node({})[\"{}\"];", center, w.key);
            } else {
                // Escape quotes in value if necessary (simple approach)
                query += std::format("node({})[\"{}\"=\"{}\"];", center, w.key, w.value);
            }
        }
    }

    query += ");out center;";
    return query;
}

nlohmann::json PoiOsmClient::buildResultJson_(
    double centerLat, double centerLon,
    int radiusMeters,
    const std::vector<PoiWhitelistEntry>& whitelist,
    const nlohmann::json& elements,
    const nlohmann::json& queryInput) const {

    nlohmann::json root;
    root["schema_version"] = 1;

    nlohmann::json source;
    source["provider"] = "OpenStreetMap";
    source["geocoder"] = "Nominatim";
    source["overpass_endpoint"] = "https://overpass-api.de/api/interpreter";
    root["source"] = source;

    nlohmann::json query;
    query["input"] = queryInput;

    nlohmann::json resolvedCenter;
    resolvedCenter["lat"] = centerLat;
    resolvedCenter["lon"] = centerLon;
    query["resolved_center"] = resolvedCenter;
    query["radius_m"] = radiusMeters;

    nlohmann::json wlArray = nlohmann::json::array();
    for (const auto& w : whitelist) {
        wlArray.push_back({{"key", w.key}, {"value", w.value}});
    }
    query["whitelist"] = wlArray;
    query["timestamp_utc"] = currentIsoTime();
    root["query"] = query;

    nlohmann::json poisArray = nlohmann::json::array();
    for (const auto& obj : elements) {
        if (obj.value("type", "") != "node") continue;

        double lat = obj.value("lat", 0.0);
        double lon = obj.value("lon", 0.0);
        nlohmann::json tags = obj.value("tags", nlohmann::json::object());

        bool accepted = whitelist.empty();
        if (!accepted) {
            for (const auto& w : whitelist) {
                if (tags.contains(w.key)) {
                    std::string val = tags[w.key];
                    if (w.value.empty() || val == w.value) {
                        accepted = true;
                        break;
                    }
                }
            }
        }

        if (!accepted) continue;

        nlohmann::json poi;
        poi["lat"] = lat;
        poi["lon"] = lon;
        
        if (tags.contains("name")) {
            poi["name"] = tags["name"];
        } else {
            poi["name"] = nullptr;
        }

        poi["tags"] = tags;
        poisArray.push_back(poi);
    }

    nlohmann::json results;
    results["count"] = poisArray.size();
    results["pois"] = poisArray;
    root["results"] = results;

    return root;
}
