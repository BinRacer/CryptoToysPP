#!/usr/bin/env python3
# ==============================================================================
# CryptoToysPP Build Output Copier (Optimized)
#   - Only copies executables and dependencies to dist directory
#   - Cross-platform support
#   - Clears target directory before copying in both Debug and Release modes
# ==============================================================================
import os
import sys
import shutil
import platform
import argparse
import logging
from pathlib import Path

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='[%(levelname)s] %(message)s',
    stream=sys.stdout
)
logger = logging.getLogger('build_copier')

def copy_build_output(source_dir, build_type):
    """
    Copy build artifacts to dist directory

    Args:
        source_dir: Path to build output directory
        build_type: Build configuration (Debug/Release)
    """
    try:
        # Determine project root (script directory parent)
        script_dir = Path(__file__).resolve().parent
        project_root = script_dir.parent
        target_dir = project_root / "dist" / build_type

        logger.info(f"Project root: {project_root}")
        logger.info(f"Source directory: {source_dir}")
        logger.info(f"Target directory: {target_dir}")

        # Clear target directory for both Debug and Release modes
        if target_dir.exists():
            logger.info(f"Clearing target directory for {build_type} build: {target_dir}")
            shutil.rmtree(target_dir)

        # Ensure target directory exists
        target_dir.mkdir(parents=True, exist_ok=True)

        # Platform-specific executable name
        executable_name = "CryptoToysPP"
        if platform.system() == "Windows":
            executable_name += ".exe"

        # Copy executable
        source_exe = Path(source_dir) / executable_name
        target_exe = target_dir / executable_name

        if source_exe.exists():
            logger.info(f"Copying executable: {source_exe} → {target_exe}")
            shutil.copy2(source_exe, target_exe)
        else:
            logger.warning(f"Missing executable: {source_exe}")

        # Copy all platform-specific dependencies
        copy_platform_dependencies(source_dir, target_dir)

        logger.info(f"Successfully copied artifacts to dist/{build_type}")
        return True
    except Exception as e:
        logger.error(f"Operation failed: {str(e)}")
        return False

def copy_platform_dependencies(source_dir, target_dir):
    """Copy platform-specific dependencies"""
    system = platform.system()
    source_path = Path(source_dir)

    if system == "Windows":
        # Copy all DLL files
        for dll in source_path.glob("*.dll"):
            target_dll = target_dir / dll.name
            logger.info(f"Copying DLL: {dll} → {target_dll}")
            shutil.copy2(dll, target_dll)

    elif system == "Darwin":
        # Copy dynamic libraries
        for dylib in source_path.glob("*.dylib"):
            target_dylib = target_dir / dylib.name
            logger.info(f"Copying dylib: {dylib} → {target_dylib}")
            shutil.copy2(dylib, target_dylib)

        # Copy frameworks
        for framework in source_path.glob("*.framework"):
            target_framework = target_dir / framework.name
            logger.info(f"Copying framework: {framework} → {target_framework}")
            shutil.copytree(framework, target_framework, dirs_exist_ok=True)

    elif system == "Linux":
        # Copy shared libraries (.so files)
        for so_file in source_path.glob("*.so*"):
            target_so = target_dir / so_file.name
            logger.info(f"Copying shared library: {so_file} → {target_so}")
            shutil.copy2(so_file, target_so)

def main():
    parser = argparse.ArgumentParser(
        description="CryptoToysPP Build Output Copier - Copies executables and dependencies",
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
        logger.error(f"Invalid source directory: {args.source_dir}")
        sys.exit(1)

    # Execute copy operation
    success = copy_build_output(args.source_dir, args.build_type)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
