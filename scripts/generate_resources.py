import os
import sys
from pathlib import Path

def main():
    root_dir = Path(__file__).parent.parent
    ui_dir = root_dir / "ui"
    header_path = root_dir / "src/resources.h"
    cpp_path = root_dir / "src/resources.cpp"  # 新增.cpp文件存储数据

    # 确保输出目录存在
    header_path.parent.mkdir(parents=True, exist_ok=True)
    cpp_path.parent.mkdir(parents=True, exist_ok=True)

    resource_map = {}
    resource_data = []
    current_offset = 0

    # 收集所有UI文件并按路径排序保证稳定性
    ui_files = []
    for root, _, files in os.walk(ui_dir):
        for file in files:
            full_path = os.path.join(root, file)
            rel_path = os.path.relpath(full_path, ui_dir)
            posix_path = f"/{rel_path.replace('\\', '/')}"
            ui_files.append((posix_path, full_path))

    # 按路径排序保证数据顺序一致性
    ui_files.sort(key=lambda x: x[0])

    for posix_path, full_path in ui_files:
        # 读取文件内容
        with open(full_path, 'rb') as f:
            content = f.read()

        # 添加到数据池并记录位置/长度
        resource_data.append(content)
        resource_map[posix_path] = (current_offset, len(content))
        current_offset += len(content)

    # 生成.cpp资源数据文件
    with open(cpp_path, 'w', encoding='utf-8', newline='\n') as cpp:
        cpp.write('#include "resources.h"\n\n')
        cpp.write('namespace {\n')
        cpp.write('    // 编译期初始化的资源数据池\n')
        cpp.write('    constexpr uint8_t RESOURCE_DATA_ARRAY[] = {\n')

        # 写入所有资源的字节数据
        byte_counter = 0
        for data in resource_data:
            for byte in data:
                # 每16字节换行
                if byte_counter % 16 == 0:
                    cpp.write(f'        ')

                cpp.write(f'0x{byte:02X}, ')
                byte_counter += 1

                # 处理换行
                if byte_counter % 16 == 0:
                    cpp.write('\n')

        # 处理最后一行
        if byte_counter % 16 != 0:
            cpp.write('\n')
        cpp.write('    };\n}\n\n')

        # 在头文件中声明数据结构
        cpp.write('const resources::ResourceData resources::RESOURCE_DATA = {\n')
        cpp.write('    .data = RESOURCE_DATA_ARRAY,\n')
        cpp.write(f'    .size = {current_offset}\n')
        cpp.write('};\n')

    # 生成头文件
    with open(header_path, 'w', encoding='utf-8', newline='\n') as header:
        header.write("#pragma once\n")
        header.write("#include <cstdint>\n")
        header.write("#include <unordered_map>\n")
        header.write("#include <string>\n")
        header.write("#include <utility>\n\n")

        # 声明命名空间防止冲突
        header.write("namespace resources {\n\n")

        # 定义数据结构
        header.write("struct ResourceData {\n")
        header.write("    const uint8_t* data;  // 资源数据指针\n")
        header.write("    const size_t size;     // 总数据大小\n")
        header.write("};\n\n")

        # 声明外部常量
        header.write("// 编译期初始化的资源数据\n")
        header.write("extern const ResourceData RESOURCE_DATA;\n\n")

        header.write("// 资源路径到(偏移, 长度)的映射\n")
        header.write("inline const std::unordered_map<std::string, std::pair<size_t, size_t>> RESOURCE_MAP = {\n")

        # 写入资源映射表
        for path, (offset, length) in resource_map.items():
            safe_path = path.replace('"', '\\"')
            header.write(f'    {{ "{safe_path}", {{ {offset}, {length} }} }},\n')

        header.write("};\n\n")
        header.write("} // namespace resources\n")

    print(f"资源头文件已生成: {header_path}")
    print(f"资源数据文件已生成: {cpp_path}")

if __name__ == "__main__":
    main()
