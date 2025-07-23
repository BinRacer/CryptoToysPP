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
        { "/assets/crypto-CODKt67F.js", { 0, 66672 } },
        { "/assets/crypto-DTvxxSna.css", { 66672, 4516 } },
        { "/assets/element-plus-B2F6cF_Y.js", { 71188, 719673 } },
        { "/assets/element-plus-BIHauc6T.css", { 790861, 338932 } },
        { "/assets/element-plus-icons-D0qR3wG7.js", { 1129793, 24640 } },
        { "/assets/index-Xrq_apr9.css", { 1154433, 385 } },
        { "/assets/index-iUx_q_zN.js", { 1154818, 13556 } },
        { "/assets/lodash-vendor-S0Y0Up6J.js", { 1168374, 26859 } },
        { "/assets/vendor-wFbPd71c.js", { 1195233, 87707 } },
        { "/assets/vue-C8G1xC4j.js", { 1282940, 83932 } },
        { "/assets/vueuse-BcX3i8Zw.js", { 1366872, 7867 } },
        { "/favicon.ico", { 1374739, 4286 } },
        { "/index.html", { 1379025, 1111 } },
    };
} // namespace CryptoToysPP::Resources
