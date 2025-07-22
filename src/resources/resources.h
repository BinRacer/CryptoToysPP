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
        { "/assets/crypto-DeNzEk6t.css", { 0, 4516 } },
        { "/assets/crypto-FE319QAO.js", { 4516, 66701 } },
        { "/assets/element-plus-BIHauc6T.css", { 71217, 338932 } },
        { "/assets/element-plus-Dgdwx9p7.js", { 410149, 719673 } },
        { "/assets/element-plus-icons-y9dc0lWh.js", { 1129822, 24640 } },
        { "/assets/index-D_NuDUO_.js", { 1154462, 9444 } },
        { "/assets/index-Xrq_apr9.css", { 1163906, 385 } },
        { "/assets/lodash-vendor-S0Y0Up6J.js", { 1164291, 26859 } },
        { "/assets/vendor-wFbPd71c.js", { 1191150, 87707 } },
        { "/assets/vue-DRjelvjH.js", { 1278857, 83932 } },
        { "/assets/vueuse-iYfs8jeO.js", { 1362789, 7867 } },
        { "/favicon.ico", { 1370656, 4286 } },
        { "/index.html", { 1374942, 1111 } },
    };
} // namespace CryptoToysPP::Resources
