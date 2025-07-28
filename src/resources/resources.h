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
        { "/assets/crypto-CZo2ejW6.js", { 0, 87553 } },
        { "/assets/crypto-DXlUDT98.css", { 87553, 59060 } },
        { "/assets/element-plus-BIHauc6T.css", { 146613, 338932 } },
        { "/assets/element-plus-icons-CHcYaXag.js", { 485545, 27848 } },
        { "/assets/element-plus-qJvjUKHo.js", { 513393, 719673 } },
        { "/assets/index-BPCOBTTq.js", { 1233066, 21329 } },
        { "/assets/index-BaoqhA7H.css", { 1254395, 11589 } },
        { "/assets/lodash-vendor-S0Y0Up6J.js", { 1265984, 26859 } },
        { "/assets/vendor-wFbPd71c.js", { 1292843, 87707 } },
        { "/assets/vue-C1fXg_7f.js", { 1380550, 83837 } },
        { "/favicon.ico", { 1464387, 4286 } },
        { "/index.html", { 1468673, 888 } },
    };
} // namespace CryptoToysPP::Resources
