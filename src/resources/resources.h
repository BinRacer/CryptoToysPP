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
        { "/assets/crypto-D7wwl_IU.js", { 0, 87741 } },
        { "/assets/crypto-DCkmU1ur.css", { 87741, 52969 } },
        { "/assets/element-plus-BIHauc6T.css", { 140710, 338932 } },
        { "/assets/element-plus-DxwSQPBo.js", { 479642, 719673 } },
        { "/assets/element-plus-icons-BYh6yX7U.js", { 1199315, 27665 } },
        { "/assets/index-BRBVknHj.css", { 1226980, 10130 } },
        { "/assets/index-Xy8YjHE7.js", { 1237110, 19043 } },
        { "/assets/lodash-vendor-S0Y0Up6J.js", { 1256153, 26859 } },
        { "/assets/vendor-wFbPd71c.js", { 1283012, 87707 } },
        { "/assets/vue-DLLgnhVy.js", { 1370719, 83842 } },
        { "/favicon.ico", { 1454561, 4286 } },
        { "/index.html", { 1458847, 888 } },
    };
} // namespace CryptoToysPP::Resources
