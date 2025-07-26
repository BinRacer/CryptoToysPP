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
        { "/assets/crypto-BhsL09-9.css", { 0, 56706 } },
        { "/assets/crypto-DoVc5HiG.js", { 56706, 93791 } },
        { "/assets/element-plus-BIHauc6T.css", { 150497, 338932 } },
        { "/assets/element-plus-icons-BcHgeD8x.js", { 489429, 27665 } },
        { "/assets/element-plus-ktg67pxk.js", { 517094, 719673 } },
        { "/assets/index-B46jJeYT.js", { 1236767, 19042 } },
        { "/assets/index-BRBVknHj.css", { 1255809, 10130 } },
        { "/assets/lodash-vendor-S0Y0Up6J.js", { 1265939, 26859 } },
        { "/assets/vendor-wFbPd71c.js", { 1292798, 87707 } },
        { "/assets/vue-BIzKO8mN.js", { 1380505, 83842 } },
        { "/favicon.ico", { 1464347, 4286 } },
        { "/index.html", { 1468633, 888 } },
    };
} // namespace CryptoToysPP::Resources
