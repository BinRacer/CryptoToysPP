#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>
#include <utility>

namespace CryptoToysPP::Resources {
    struct ResourceData {
        const uint8_t* data;   // 资源数据指针
        const size_t size;     // 总数据大小
    };

    // 编译期初始化的资源数据
    extern const ResourceData RESOURCE_DATA;

    // 资源路径到(偏移, 长度)的映射
    inline const std::unordered_map<std::string, std::pair<size_t, size_t>> RESOURCE_MAP = {
        { "/assets/crypto-CjxDkm3U.css", { 0, 60819 } },
        { "/assets/crypto-DiBKBJvZ.js", { 60819, 86422 } },
        { "/assets/element-plus-BIHauc6T.css", { 147241, 338932 } },
        { "/assets/element-plus-BioA0Pw6.js", { 486173, 719673 } },
        { "/assets/element-plus-icons-86yR2AJb.js", { 1205846, 27283 } },
        { "/assets/index-C26GcD2F.js", { 1233129, 17318 } },
        { "/assets/index-jQgLRGgY.css", { 1250447, 8928 } },
        { "/assets/lodash-vendor-S0Y0Up6J.js", { 1259375, 26859 } },
        { "/assets/vendor-wFbPd71c.js", { 1286234, 87707 } },
        { "/assets/vue-BuFDwaAe.js", { 1373941, 83837 } },
        { "/favicon.ico", { 1457778, 4286 } },
        { "/index.html", { 1462064, 888 } },
    };
} // namespace CryptoToysPP::Resources
