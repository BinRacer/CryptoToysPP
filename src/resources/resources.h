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
        { "/assets/crypto-C9Kn8QjX.css", { 0, 25602 } },
        { "/assets/crypto-DRcBy9IZ.js", { 25602, 69170 } },
        { "/assets/element-plus-BIHauc6T.css", { 94772, 338932 } },
        { "/assets/element-plus-DxwSQPBo.js", { 433704, 719673 } },
        { "/assets/element-plus-icons-BYh6yX7U.js", { 1153377, 27665 } },
        { "/assets/index-BRBVknHj.css", { 1181042, 10130 } },
        { "/assets/index-BwLHjsOG.js", { 1191172, 17245 } },
        { "/assets/lodash-vendor-S0Y0Up6J.js", { 1208417, 26859 } },
        { "/assets/vendor-wFbPd71c.js", { 1235276, 87707 } },
        { "/assets/vue-DLLgnhVy.js", { 1322983, 83842 } },
        { "/favicon.ico", { 1406825, 4286 } },
        { "/index.html", { 1411111, 888 } },
    };
} // namespace CryptoToysPP::Resources
