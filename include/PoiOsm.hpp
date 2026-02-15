/**
 * SPDX-FileComment: Header file for OpenStreetMap POI client
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file PoiOsm.hpp
 * @brief Defines the PoiOsmClient class for querying OpenStreetMap Overpass
 * API.
 * @version 0.1.0
 * @date 2026-02-15
 *
 * @author ZHENG Robert
 * @license MIT License
 */

#pragma once

#include <expected>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

/**
 * @brief Represents a whitelist entry for filtering POIs.
 *
 * A whitelist entry consists of a key (e.g., "amenity") and an optional value
 * (e.g., "restaurant"). If the value is empty, any POI with the key is
 * accepted.
 */
struct PoiWhitelistEntry {
    std::string key;   ///< The tag key (e.g., "amenity").
    std::string value; ///< The tag value (optional).
};

/**
 * @brief Client for querying Points of Interest (POIs) from OpenStreetMap.
 *
 * This class provides methods to query POIs either by address (using Nominatim
 * for geocoding) or by direct geographic coordinates (using Overpass API).
 */
class PoiOsmClient {
public:
    /**
     * @brief Queries POIs around a specific address.
     *
     * Encodes the address using Nominatim and then queries the Overpass API
     * around the resolved coordinates.
     *
     * @param address The address to search for.
     * @param radiusMeters The search radius in meters.
     * @param whitelist List of key-value pairs to filter results.
     * @return std::expected<nlohmann::json, std::string> The result JSON or an error message.
     */
    std::expected<nlohmann::json, std::string> queryByAddress(
        const std::string& address, int radiusMeters,
        const std::vector<PoiWhitelistEntry>& whitelist = {});

    /**
     * @brief Queries POIs around specific geographic coordinates.
     *
     * Queries the Overpass API directly using the provided coordinates.
     *
     * @param lat Latitude of the center point.
     * @param lon Longitude of the center point.
     * @param radiusMeters The search radius in meters.
     * @param whitelist List of key-value pairs to filter results.
     * @return std::expected<nlohmann::json, std::string> The result JSON or an error message.
     */
    std::expected<nlohmann::json, std::string> queryByCoordinates(
        double lat, double lon, int radiusMeters,
        const std::vector<PoiWhitelistEntry>& whitelist = {});

private:
    /**
     * @brief Internal helper to geocode an address.
     *
     * @param address The address to geocode.
     * @return std::expected<std::pair<double, double>, std::string> The coordinates (lat, lon) or error.
     */
    std::expected<std::pair<double, double>, std::string> geocodeAddress_(const std::string& address);

    /**
     * @brief Internal helper to perform the Overpass API query.
     *
     * @param lat Latitude.
     * @param lon Longitude.
     * @param radiusMeters Search radius.
     * @param whitelist Filter list.
     * @param queryInput The original query input (for result JSON construction).
     * @return std::expected<nlohmann::json, std::string> The result JSON or error.
     */
    std::expected<nlohmann::json, std::string> queryOverpass_(
        double lat, double lon, int radiusMeters,
        const std::vector<PoiWhitelistEntry>& whitelist,
        const nlohmann::json& queryInput);

    /**
     * @brief Builds the Overpass QL query string.
     *
     * @param lat Latitude.
     * @param lon Longitude.
     * @param radiusMeters Search radius.
     * @param whitelist Filter list.
     * @return std::string The formatted Overpass QL query.
     */
    std::string buildOverpassQuery_(
        double lat, double lon, int radiusMeters,
        const std::vector<PoiWhitelistEntry>& whitelist) const;

    /**
     * @brief Constructs the final JSON result object.
     *
     * @param centerLat Latitude of the search center.
     * @param centerLon Longitude of the search center.
     * @param radiusMeters Search radius.
     * @param whitelist Filter list used.
     * @param elements The raw elements array from Overpass.
     * @param queryInput The input parameters.
     * @return nlohmann::json The structured result JSON.
     */
    nlohmann::json buildResultJson_(
        double centerLat, double centerLon,
        int radiusMeters,
        const std::vector<PoiWhitelistEntry>& whitelist,
        const nlohmann::json& elements,
        const nlohmann::json& queryInput) const;
};
