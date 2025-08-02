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
    Copy build artifacts to dist directory with enhanced macOS app bundle support

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

        # Handle macOS app bundle specifically
        if platform.system() == "Darwin":
            return handle_macos_bundle(source_dir, target_dir, build_type)

        # Non-macOS platforms
        return handle_other_platforms(source_dir, target_dir, build_type)

    except Exception as e:
        logger.error(f"Operation failed: {str(e)}", exc_info=True)
        return False

def handle_macos_bundle(source_dir, target_dir, build_type):
    """Handle macOS app bundle copy with enhanced detection"""
    app_bundle_name = "CryptoToysPP.app"

    # Check if source_dir is the app bundle itself
    if source_dir.endswith(".app") and Path(source_dir).is_dir():
        source_bundle = Path(source_dir)
        logger.info(f"Found macOS app bundle at: {source_bundle}")
    else:
        # Check common locations for app bundle
        possible_paths = [
            Path(source_dir).parent / app_bundle_name,  # Build root directory
            Path(source_dir).parent.parent / app_bundle_name,  # Build configuration directory
            Path(source_dir) / app_bundle_name,  # Executable directory
        ]

        # Try to find the app bundle in possible locations
        source_bundle = None
        for path in possible_paths:
            if path.exists() and path.is_dir():
                source_bundle = path
                logger.info(f"Found macOS app bundle at: {source_bundle}")
                break

    # If found, copy the bundle
    if source_bundle:
        target_bundle = target_dir / app_bundle_name
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

    # Bundle not found - try fallback to executable copy
    logger.warning("App bundle not found, falling back to executable copy")
    executable_name = "CryptoToysPP"
    source_exe = Path(source_dir) / executable_name

    if source_exe.exists():
        target_exe = target_dir / executable_name
        logger.info(f"Copying executable: {source_exe} → {target_exe}")
        shutil.copy2(source_exe, target_exe)

        # Copy dependencies
        copy_platform_dependencies(source_dir, target_dir, build_type)
        logger.info(f"Successfully copied artifacts to dist/{build_type}")
        return True

    # Everything failed
    logger.error(f"❌ macOS app bundle or executable not found")
    logger.error("Searched paths:")
    for path in possible_paths:
        logger.error(f"  - {path.resolve()}")

    return False

def handle_other_platforms(source_dir, target_dir, build_type):
    """Handle copy for Windows and Linux platforms"""
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
    copy_platform_dependencies(source_dir, target_dir, build_type)

    logger.info(f"Successfully copied artifacts to dist/{build_type}")
    return True

def copy_platform_dependencies(source_dir, target_dir, build_type):
    """Copy platform-specific dependencies"""
    system = platform.system()
    source_path = Path(source_dir)

    # Create lib directory for all platforms
    lib_dir = target_dir / "lib"
    lib_dir.mkdir(exist_ok=True)

    if system == "Windows":
        # Copy all DLL files
        for dll in source_path.glob("*.dll"):
            target_dll = target_dir / dll.name
            logger.info(f"Copying DLL: {dll} → {target_dll}")
            shutil.copy2(dll, target_dll)

        # Copy libs to lib directory
        for lib in source_path.glob("*.lib"):
            target_lib = lib_dir / lib.name
            logger.info(f"Copying LIB: {lib} → {target_lib}")
            shutil.copy2(lib, target_lib)

    elif system == "Darwin":
        # Copy dynamic libraries (.dylib files)
        for dylib in source_path.glob("*.dylib*"):
            # Skip symlinks in this case - they're handled by the bundle
            if not dylib.is_symlink():
                target_dylib = lib_dir / dylib.name
                logger.info(f"Copying dylib: {dylib} → {target_dylib}")
                shutil.copy2(dylib, target_dylib)

    elif system == "Linux":
        # Copy shared libraries (.so files)
        for so_file in source_path.glob("*.so*"):
            # Copy versioned libraries to lib directory
            target_so = lib_dir / so_file.name if so_file.name.endswith((".so", ".so.*")) else target_dir / so_file.name
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