import os
import sys
from pathlib import Path

def main():
    root_dir = Path(__file__).parent.parent
    ui_dir = root_dir / "ui"
    header_path = root_dir / "src/resources.h"

    # 确保输出目录存在
    header_path.parent.mkdir(parents=True, exist_ok=True)

    resource_map = {}
    for root, _, files in os.walk(ui_dir):
        for file in files:
            # 获取相对路径并转换为POSIX格式
            rel_path = os.path.relpath(os.path.join(root, file), ui_dir)
            # 添加前导斜杠作为统一路径前缀
            posix_path = f"/{rel_path.replace('\\', '/')}"

            # 读取文件内容
            with open(os.path.join(root, file), 'rb') as f:
                content = f.read()

            # 将字节转换为十六进制字符串表示，每256个字节添加换行符
            hex_lines = []
            line_break = 256  # 每256字节换行
            chunk_count = (len(content) + line_break - 1) // line_break

            for i in range(chunk_count):
                start = i * line_break
                end = min((i + 1) * line_break, len(content))
                chunk = content[start:end]
                hex_line = ", ".join(f'0x{b:02X}' for b in chunk)
                hex_lines.append(hex_line)

            # 连接所有行，添加缩进
            formatted_hex = ",\n            ".join(hex_lines)
            resource_map[posix_path] = f'{{\n            {formatted_hex}\n        }}'

    # 关键更新：使用LF换行和UTF-8无BOM编码
    with open(header_path, 'w', encoding='utf-8', newline='\n') as f:
        f.write("#pragma once\n")
        f.write("#include <unordered_map>\n")
        f.write("#include <string>\n")
        f.write("#include <cstdint>\n")
        f.write("#include <vector>\n\n")
        f.write("inline const std::unordered_map<std::string, std::vector<uint8_t>> RESOURCE_MAP = {\n")

        # 写入每个资源条目
        for path, data in resource_map.items():
            # 路径转义处理
            safe_path = path.replace('"', '\\"')
            f.write(f'    {{ "{safe_path}", std::vector<uint8_t>{data} }},\n')

        f.write("};\n")

    print(f"资源头文件已生成: {header_path} (UTF-8无BOM, LF换行)")

if __name__ == "__main__":
    main()
