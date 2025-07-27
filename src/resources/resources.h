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
        { "/assets/crypto-CGq4fC7_.js", { 0, 87553 } },
        { "/assets/crypto-DXlUDT98.css", { 87553, 59060 } },
        { "/assets/element-plus-BIHauc6T.css", { 146613, 338932 } },
        { "/assets/element-plus-XIuufz55.js", { 485545, 719673 } },
        { "/assets/element-plus-icons-BjQqgfdT.js", { 1205218, 27665 } },
        { "/assets/index-5I4X7zwV.js", { 1232883, 19042 } },
        { "/assets/index-BRBVknHj.css", { 1251925, 10130 } },
        { "/assets/lodash-vendor-S0Y0Up6J.js", { 1262055, 26859 } },
        { "/assets/vendor-wFbPd71c.js", { 1288914, 87707 } },
        { "/assets/vue-C1fXg_7f.js", { 1376621, 83837 } },
        { "/favicon.ico", { 1460458, 4286 } },
        { "/index.html", { 1464744, 888 } },
    };
} // namespace CryptoToysPP::Resources
