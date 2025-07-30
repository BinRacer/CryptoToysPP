"""
Build Output Copier
===================
Copies build artifacts to project build directory
with structure:
  build/
    ├── Debug/
    └── Release/
"""

import os
import shutil
import sys
import platform
import argparse
import glob

def copy_build_output(source_dir, build_type):
    """
    Copy build artifacts to project build directory

    Args:
        source_dir: Path to build output directory
        build_type: Build configuration (Debug/Release)
    """
    # Determine project root (script directory parent)
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    target_dir = os.path.join(project_root, "build", build_type)

    print(f"[INFO] Project root: {project_root}")
    print(f"[INFO] Source directory: {source_dir}")
    print(f"[INFO] Target directory: {target_dir}")

    # Ensure target directory exists
    os.makedirs(target_dir, exist_ok=True)

    # Platform-specific executable name
    executable_name = "CryptoToysPP"
    if platform.system() == "Windows":
        executable_name += ".exe"

    # Copy executable
    source_exe = os.path.join(source_dir, executable_name)
    target_exe = os.path.join(target_dir, executable_name)

    if os.path.exists(source_exe):
        print(f"[COPY] Executable: {source_exe} → {target_exe}")
        shutil.copy2(source_exe, target_exe)
    else:
        print(f"[WARNING] Missing executable: {source_exe}")

    # Platform-specific dependencies
    if platform.system() == "Windows":
        # Copy all DLL files
        dll_files = glob.glob(os.path.join(source_dir, "*.dll"))
        for dll in dll_files:
            target_dll = os.path.join(target_dir, os.path.basename(dll))
            print(f"[COPY] DLL: {dll} → {target_dll}")
            shutil.copy2(dll, target_dll)

    elif platform.system() == "Darwin":
        # Copy dynamic libraries
        dylib_files = glob.glob(os.path.join(source_dir, "*.dylib"))
        for dylib in dylib_files:
            target_dll = os.path.join(target_dir, os.path.basename(dylib))
            print(f"[COPY] dylib: {dylib} → {target_dll}")
            shutil.copy2(dylib, target_dll)

        # Copy frameworks
        frameworks = glob.glob(os.path.join(source_dir, "*.framework"))
        for framework in frameworks:
            framework_name = os.path.basename(framework)
            target_framework = os.path.join(target_dir, framework_name)

            # Remove existing framework directory
            if os.path.exists(target_framework):
                shutil.rmtree(target_framework)

            print(f"[COPY] Framework: {framework} → {target_framework}")
            shutil.copytree(framework, target_framework)

    # Copy configuration files
    config_patterns = ("*.ini", "*.conf", "*.cfg")
    for pattern in config_patterns:
        for config in glob.glob(os.path.join(source_dir, pattern)):
            target_config = os.path.join(target_dir, os.path.basename(config))
            print(f"[COPY] Config: {config} → {target_config}")
            shutil.copy2(config, target_config)

    print(f"[SUCCESS] Copied artifacts to build/{build_type}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Copy build output to project build directory",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )

    parser.add_argument(
        "source_dir",
        help="Build output directory path"
    )

    parser.add_argument(
        "build_type",
        choices=["Debug", "Release"],
        help="Build configuration"
    )

    args = parser.parse_args()

    # Validate source directory
    if not os.path.exists(args.source_dir):
        print(f"[ERROR] Invalid source directory: {args.source_dir}")
        sys.exit(1)

    # Execute copy operation
    try:
        copy_build_output(args.source_dir, args.build_type)
    except Exception as e:
        print(f"[ERROR] Operation failed: {str(e)}")
        sys.exit(1)
