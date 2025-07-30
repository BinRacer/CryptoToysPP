#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>
#include <utility>

namespace CryptoToysPP::Resources {
    // Structure for resource data reference
    struct ResourceData {
        const uint8_t* data;   // Pointer to resource bytes
        const size_t size;     // Total resource size
    };

    // Global resource data instance
    extern const ResourceData RESOURCE_DATA;

    // Mapping from resource path to (offset, length)
    inline const std::unordered_map<std::string, std::pair<size_t, size_t>> RESOURCE_MAP = {
        { "/assets/crypto-CMO81S4n.js", { 0, 86520 } },
        { "/assets/crypto-DfQlMEvu.css", { 86520, 60819 } },
        { "/assets/element-plus-BIHauc6T.css", { 147339, 338932 } },
        { "/assets/element-plus-BioA0Pw6.js", { 486271, 719673 } },
        { "/assets/element-plus-icons-86yR2AJb.js", { 1205944, 27283 } },
        { "/assets/index-cb5m26cP.js", { 1233227, 17318 } },
        { "/assets/index-jQgLRGgY.css", { 1250545, 8928 } },
        { "/assets/lodash-vendor-S0Y0Up6J.js", { 1259473, 26859 } },
        { "/assets/vendor-wFbPd71c.js", { 1286332, 87707 } },
        { "/assets/vue-BuFDwaAe.js", { 1374039, 83837 } },
        { "/favicon.ico", { 1457876, 4286 } },
        { "/index.html", { 1462162, 888 } },
    };
} // namespace CryptoToysPP::Resources
