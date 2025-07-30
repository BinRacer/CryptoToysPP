"""
Modern Resource Generator
=========================
Optimized version preserving original C++ resource generation logic
Generates:
  src/resources/
    ├── resources.h
    └── resources.cpp
"""

import os
import sys
import argparse
from pathlib import Path
from typing import Dict, Tuple, Generator


def collect_ui_files(ui_dir: Path) -> Generator[Tuple[str, Path], None, None]:
    """Collect UI files with deterministic path ordering"""
    for root, _, files in os.walk(ui_dir):
        for file in files:
            file_path = Path(root) / file
            rel_path = os.path.relpath(file_path, ui_dir)
            yield f"/{rel_path.replace('\\', '/')}", file_path


def create_header_file(header_path: Path,
                       resource_map: Dict[str, Tuple[int, int]]) -> None:
    """Generate header file with original C++ logic preserved"""
    with header_path.open('w', encoding='utf-8', newline='\n') as f:
        f.write("#pragma once\n")
        f.write("#include <cstdint>\n")
        f.write("#include <unordered_map>\n")
        f.write("#include <string>\n")
        f.write("#include <utility>\n\n")
        f.write("namespace CryptoToysPP::Resources {\n")
        f.write("    // Structure for resource data reference\n")
        f.write("    struct ResourceData {\n")
        f.write("        const uint8_t* data;   // Pointer to resource bytes\n")
        f.write("        const size_t size;     // Total resource size\n")
        f.write("    };\n\n")
        f.write("    // Global resource data instance\n")
        f.write("    extern const ResourceData RESOURCE_DATA;\n\n")
        f.write("    // Mapping from resource path to (offset, length)\n")
        f.write("    inline const std::unordered_map<std::string, std::pair<size_t, size_t>> RESOURCE_MAP = {\n")

        for path, (offset, length) in resource_map.items():
            safe_path = path.replace('"', '\\"')
            f.write(f'        {{ "{safe_path}", {{ {offset}, {length} }} }},\n')

        f.write("    };\n")
        f.write("} // namespace CryptoToysPP::Resources\n")


def create_cpp_file(cpp_path: Path,
                    resource_data: bytes) -> None:
    """Generate implementation file with original C++ logic preserved"""
    with cpp_path.open('w', encoding='utf-8', newline='\n') as f:
        f.write('#include "resources.h"\n\n')
        f.write('namespace CryptoToysPP::Resources {\n')
        f.write('    // Compile-time initialized resource byte array\n')
        f.write('    constexpr uint8_t RESOURCE_DATA_ARRAY[] = {\n')

        # Preserve original byte formatting
        byte_count = 0
        for byte in resource_data:
            if byte_count % 16 == 0:
                f.write('        ')
            f.write(f'0x{byte:02X}, ')
            byte_count += 1
            if byte_count % 16 == 0:
                f.write('\n')

        if byte_count % 16 != 0:
            f.write('\n')
        f.write('    };\n\n')
        f.write('    // Global resource data instance initialization\n')
        f.write('    const ResourceData RESOURCE_DATA = {\n')
        f.write('        .data = RESOURCE_DATA_ARRAY,\n')
        f.write(f'        .size = {len(resource_data)}\n')
        f.write('    };\n')
        f.write('} // namespace CryptoToysPP::Resources\n')


def generate_resources(ui_dir: Path, header_path: Path, cpp_path: Path) -> None:
    """Core resource generation with enhanced logging and validation"""
    # Validate UI directory
    if not ui_dir.is_dir():
        print(f"[ERROR] UI directory not found: {ui_dir}")
        sys.exit(1)

    # Ensure output directories exist
    for path in [header_path.parent, cpp_path.parent]:
        path.mkdir(parents=True, exist_ok=True)
        if not path.is_dir():
            print(f"[ERROR] Cannot create output directory: {path}")
            sys.exit(1)

    # Collect and sort UI files
    print(f"[INFO] Scanning UI directory: {ui_dir}")
    ui_files = sorted(collect_ui_files(ui_dir), key=lambda x: x[0])

    if not ui_files:
        print("[WARNING] No UI files found in source directory")
        return

    # Process resources
    resource_map = {}
    resource_chunks = []
    current_offset = 0

    for resource_path, file_path in ui_files:
        print(f"[PROCESS] Reading {file_path}")
        try:
            content = file_path.read_bytes()
            file_size = len(content)

            resource_map[resource_path] = (current_offset, file_size)
            resource_chunks.append(content)
            current_offset += file_size
        except Exception as e:
            print(f"[ERROR] Failed to read {file_path}: {str(e)}")
            sys.exit(1)

    # Generate C++ files
    combined_data = b''.join(resource_chunks)
    print(f"[INFO] Generating header: {header_path}")
    create_header_file(header_path, resource_map)

    print(f"[INFO] Generating implementation: {cpp_path}")
    create_cpp_file(cpp_path, combined_data)

    print(f"[SUCCESS] Generated {len(ui_files)} resources ({len(combined_data)} bytes)")


def main():
    """Command-line interface with improved argument handling"""
    parser = argparse.ArgumentParser(
        description="Generate C++ resource files from UI assets",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )

    # Calculate default paths
    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent

    parser.add_argument(
        "--ui-dir",
        type=Path,
        default=project_root / "ui",
        help="Directory containing UI files"
    )

    parser.add_argument(
        "--header",
        dest="header_path",
        type=Path,
        default=project_root / "src" / "resources" / "resources.h",
        help="Output header file path"
    )

    parser.add_argument(
        "--cpp",
        dest="cpp_path",
        type=Path,
        default=project_root / "src" / "resources" / "resources.cpp",
        help="Output implementation file path"
    )

    args = parser.parse_args()
    generate_resources(args.ui_dir, args.header_path, args.cpp_path)


if __name__ == "__main__":
    main()
