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
        { "/assets/crypto-ByScI5_c.css", { 0, 8734 } },
        { "/assets/crypto-D0I8koU5.js", { 8734, 67193 } },
        { "/assets/element-plus-BIHauc6T.css", { 75927, 338932 } },
        { "/assets/element-plus-DxwSQPBo.js", { 414859, 719673 } },
        { "/assets/element-plus-icons-BYh6yX7U.js", { 1134532, 27665 } },
        { "/assets/index-BRBVknHj.css", { 1162197, 10130 } },
        { "/assets/index-CIsHub1B.js", { 1172327, 17245 } },
        { "/assets/lodash-vendor-S0Y0Up6J.js", { 1189572, 26859 } },
        { "/assets/vendor-wFbPd71c.js", { 1216431, 87707 } },
        { "/assets/vue-DLLgnhVy.js", { 1304138, 83842 } },
        { "/favicon.ico", { 1387980, 4286 } },
        { "/index.html", { 1392266, 888 } },
    };
} // namespace CryptoToysPP::Resources
