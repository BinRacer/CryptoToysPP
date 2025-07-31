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
        { "/assets/crypto-sAvCco0F.css", { 0, 60831 } },
        { "/assets/crypto-wHz5JnPM.js", { 60831, 124493 } },
        { "/assets/element-plus-BIHauc6T.css", { 185324, 338932 } },
        { "/assets/element-plus-C_RnptQt.js", { 524256, 719673 } },
        { "/assets/element-plus-icons-DLmbuT-0.js", { 1243929, 27283 } },
        { "/assets/index-DqiI-olP.js", { 1271212, 20505 } },
        { "/assets/index-E3L4V9yz.css", { 1291717, 9261 } },
        { "/assets/lodash-vendor-S0Y0Up6J.js", { 1300978, 26859 } },
        { "/assets/vendor-wFbPd71c.js", { 1327837, 87707 } },
        { "/assets/vue-D96eVTgE.js", { 1415544, 83837 } },
        { "/favicon.ico", { 1499381, 4286 } },
        { "/index.html", { 1503667, 888 } },
    };
} // namespace CryptoToysPP::Resources
