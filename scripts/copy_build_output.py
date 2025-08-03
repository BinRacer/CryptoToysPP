#!/usr/bin/env python3
# ==============================================================================
# CryptoToysPP Build Output Copier (Optimized)
#   - Enhanced macOS App Bundle handling
#   - Improved Debug/Release directory separation
#   - Cross-platform support
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
    Copy build artifacts to dist directory with platform-specific handling

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

        # Platform-specific handling
        system = platform.system()
        if system == "Darwin":
            return handle_macos(source_dir, target_dir, build_type)
        elif system == "Windows":
            return handle_windows(source_dir, target_dir, build_type)
        elif system == "Linux":
            return handle_linux(source_dir, target_dir, build_type)
        else:
            logger.error(f"Unsupported platform: {system}")
            return False

    except Exception as e:
        logger.error(f"Operation failed: {str(e)}", exc_info=True)
        return False

def handle_macos(source_dir, target_dir, build_type):
    """Handle macOS platform build output copying"""
    logger.info("Processing macOS build output")

    # Try to find and copy app bundle
    app_bundle_name = "CryptoToysPP.app"
    source_bundle = find_macos_bundle(source_dir, app_bundle_name)

    if source_bundle:
        return copy_macos_bundle(source_bundle, target_dir, build_type)

    # Fallback to executable copy if bundle not found
    return copy_macos_executable(source_dir, target_dir, build_type)

def find_macos_bundle(source_dir, bundle_name):
    """Locate macOS app bundle in common build locations"""
    # Check if source_dir is the app bundle itself
    if source_dir.endswith(".app") and Path(source_dir).is_dir():
        source_bundle = Path(source_dir)
        logger.info(f"Found macOS app bundle at: {source_bundle}")
        return source_bundle

    # Check common locations for app bundle
    possible_paths = [
        Path(source_dir).parent / bundle_name,  # Build root directory
        Path(source_dir).parent.parent / bundle_name,  # Build configuration directory
        Path(source_dir) / bundle_name,  # Executable directory
    ]

    # Try to find the app bundle
    for path in possible_paths:
        if path.exists() and path.is_dir():
            logger.info(f"Found macOS app bundle at: {path}")
            return path

    logger.warning("macOS app bundle not found in common locations")
    return None

def copy_macos_bundle(source_bundle, target_dir, build_type):
    """Copy macOS app bundle to target directory"""
    target_bundle = target_dir / source_bundle.name
    logger.info(f"Copying macOS app bundle: {source_bundle} → {target_bundle}")

    # Copy entire bundle with symlinks
    shutil.copytree(
        source_bundle,
        target_bundle,
        symlinks=True,
        ignore_dangling_symlinks=True
    )

    logger.info(f"App bundle successfully copied to dist/{build_type}")
    return True

def copy_macos_executable(source_dir, target_dir, build_type):
    """Fallback method for copying macOS executable and dependencies"""
    logger.warning("App bundle not found, falling back to executable copy")
    executable_name = "CryptoToysPP"
    source_exe = Path(source_dir) / executable_name

    if not source_exe.exists():
        logger.error(f"macOS executable not found: {source_exe}")
        return False

    target_exe = target_dir / executable_name
    logger.info(f"Copying executable: {source_exe} → {target_exe}")
    shutil.copy2(source_exe, target_exe)

    # Copy macOS dependencies
    copy_macos_dependencies(source_dir, target_dir)

    logger.info(f"Successfully copied macOS artifacts to dist/{build_type}")
    return True

def copy_macos_dependencies(source_dir, target_dir):
    """Copy macOS-specific dependencies (.dylib files)"""
    lib_dir = target_dir / "lib"
    lib_dir.mkdir(exist_ok=True)

    source_path = Path(source_dir)
    for dylib in source_path.glob("*.dylib*"):
        # Skip symlinks
        if not dylib.is_symlink():
            target_dylib = lib_dir / dylib.name
            logger.info(f"Copying dylib: {dylib} → {target_dylib}")
            shutil.copy2(dylib, target_dylib)

def handle_windows(source_dir, target_dir, build_type):
    """Handle Windows platform build output copying"""
    logger.info("Processing Windows build output")

    executable_name = "CryptoToysPP.exe"
    source_exe = Path(source_dir) / executable_name

    if not source_exe.exists():
        logger.error(f"Windows executable not found: {source_exe}")
        return False

    # Copy executable
    target_exe = target_dir / executable_name
    logger.info(f"Copying executable: {source_exe} → {target_exe}")
    shutil.copy2(source_exe, target_exe)

    # Copy Windows dependencies
    copy_windows_dependencies(source_dir, target_dir)

    logger.info(f"Successfully copied Windows artifacts to dist/{build_type}")
    return True

def copy_windows_dependencies(source_dir, target_dir):
    """Copy Windows-specific dependencies (.dll files)"""
    source_path = Path(source_dir)
    for dll in source_path.glob("*.dll"):
        target_dll = target_dir / dll.name
        logger.info(f"Copying DLL: {dll} → {target_dll}")
        shutil.copy2(dll, target_dll)

def handle_linux(source_dir, target_dir, build_type):
    """Handle Linux platform build output copying"""
    logger.info("Processing Linux build output")

    executable_name = "CryptoToysPP"
    source_exe = Path(source_dir) / executable_name

    if not source_exe.exists():
        logger.error(f"Linux executable not found: {source_exe}")
        return False

    # Copy executable
    target_exe = target_dir / executable_name
    logger.info(f"Copying executable: {source_exe} → {target_exe}")
    shutil.copy2(source_exe, target_exe)

    # Copy Linux dependencies
    copy_linux_dependencies(source_dir, target_dir)

    logger.info(f"Successfully copied Linux artifacts to dist/{build_type}")
    return True

def copy_linux_dependencies(source_dir, target_dir):
    """Copy Linux-specific dependencies (.so files)"""
    lib_dir = target_dir / "lib"
    lib_dir.mkdir(exist_ok=True)

    source_path = Path(source_dir)
    for so_file in source_path.glob("*.so*"):
        # Copy versioned libraries to lib directory
        target_so = lib_dir / so_file.name
        logger.info(f"Copying shared library: {so_file} → {target_so}")
        shutil.copy2(so_file, target_so, follow_symlinks=False)

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