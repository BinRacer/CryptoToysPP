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
        { "/assets/crypto-B_Gi3IkZ.css", { 0, 32998 } },
        { "/assets/crypto-Dm1qKYkg.js", { 32998, 70102 } },
        { "/assets/element-plus-BIHauc6T.css", { 103100, 338932 } },
        { "/assets/element-plus-DxwSQPBo.js", { 442032, 719673 } },
        { "/assets/element-plus-icons-BYh6yX7U.js", { 1161705, 27665 } },
        { "/assets/index-BRBVknHj.css", { 1189370, 10130 } },
        { "/assets/index-DmHqBp35.js", { 1199500, 17245 } },
        { "/assets/lodash-vendor-S0Y0Up6J.js", { 1216745, 26859 } },
        { "/assets/vendor-wFbPd71c.js", { 1243604, 87707 } },
        { "/assets/vue-DLLgnhVy.js", { 1331311, 83842 } },
        { "/favicon.ico", { 1415153, 4286 } },
        { "/index.html", { 1419439, 888 } },
    };
} // namespace CryptoToysPP::Resources
