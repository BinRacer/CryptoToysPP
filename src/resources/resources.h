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
        { "/assets/crypto-DZdp4s-S.css", { 0, 4516 } },
        { "/assets/crypto-DerTFy8p.js", { 4516, 66645 } },
        { "/assets/element-plus-B2F6cF_Y.js", { 71161, 719673 } },
        { "/assets/element-plus-BIHauc6T.css", { 790834, 338932 } },
        { "/assets/element-plus-icons-D0qR3wG7.js", { 1129766, 24640 } },
        { "/assets/index-CppG-i1B.js", { 1154406, 13517 } },
        { "/assets/index-Xrq_apr9.css", { 1167923, 385 } },
        { "/assets/lodash-vendor-S0Y0Up6J.js", { 1168308, 26859 } },
        { "/assets/vendor-wFbPd71c.js", { 1195167, 87707 } },
        { "/assets/vue-C8G1xC4j.js", { 1282874, 83932 } },
        { "/assets/vueuse-BcX3i8Zw.js", { 1366806, 7867 } },
        { "/favicon.ico", { 1374673, 4286 } },
        { "/index.html", { 1378959, 1111 } },
    };
} // namespace CryptoToysPP::Resources
