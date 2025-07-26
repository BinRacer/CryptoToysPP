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
        { "/assets/crypto-C33z0fqn.js", { 0, 93029 } },
        { "/assets/crypto-Cgd0O45y.css", { 93029, 56706 } },
        { "/assets/element-plus-BIHauc6T.css", { 149735, 338932 } },
        { "/assets/element-plus-icons-BcHgeD8x.js", { 488667, 27665 } },
        { "/assets/element-plus-ktg67pxk.js", { 516332, 719673 } },
        { "/assets/index-BRBVknHj.css", { 1236005, 10130 } },
        { "/assets/index-C3FOGK4q.js", { 1246135, 19042 } },
        { "/assets/lodash-vendor-S0Y0Up6J.js", { 1265177, 26859 } },
        { "/assets/vendor-wFbPd71c.js", { 1292036, 87707 } },
        { "/assets/vue-BIzKO8mN.js", { 1379743, 83842 } },
        { "/favicon.ico", { 1463585, 4286 } },
        { "/index.html", { 1467871, 888 } },
    };
} // namespace CryptoToysPP::Resources
