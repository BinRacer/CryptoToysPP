#!/bin/bash

set -euo pipefail  # Enable strict error handling

# Validate application bundle path argument
if [ $# -eq 0 ]; then
    echo "Usage: $0 /path/to/CryptoToysPP.app"
    exit 1
fi

APP_PATH="$1"
MACOS_DIR="$APP_PATH/Contents/MacOS"
FRAMEWORKS_DIR="$APP_PATH/Contents/Frameworks"

# Validate app bundle structure
if [ ! -d "$MACOS_DIR" ]; then
    echo "ERROR: Missing MacOS directory at $MACOS_DIR"
    echo "Please verify your app bundle structure"
    exit 1
fi

# Create Frameworks directory
echo "Creating Frameworks directory..."
if ! mkdir -p "$FRAMEWORKS_DIR" 2>/dev/null; then
    echo "ERROR: Failed to create Frameworks directory at $FRAMEWORKS_DIR"
    echo "Check your directory permissions"
    exit 1
fi

# Find dynamic libraries
echo "Searching for dynamic libraries..."
dylibs=()
while IFS= read -r -d '' file; do
    dylibs+=("$file")
done < <(find "$MACOS_DIR" -name "*.dylib" -print0)

if [ ${#dylibs[@]} -eq 0 ]; then
    echo "No dynamic libraries found"
    exit 0
fi

echo "Found ${#dylibs[@]} dynamic library files"
echo "Moving to Frameworks directory..."

# Move dynamic libraries to Frameworks directory
for dylib in "${dylibs[@]}"; do
    filename=$(basename "$dylib")
    echo "Moving $filename..."

    if ! mv -f "$dylib" "$FRAMEWORKS_DIR/"; then
        echo "ERROR: Failed to move $filename"
        exit 1
    fi

    # Verify after moving
    if [ ! -f "$FRAMEWORKS_DIR/$filename" ]; then
        echo "ERROR: Verification failed for $filename"
        exit 1
    fi
done

echo "Successfully moved ${#dylibs[@]} files:"
ls -l "$FRAMEWORKS_DIR"

# Update executable references
EXECUTABLE="$MACOS_DIR/CryptoToysPP"
if [ ! -f "$EXECUTABLE" ]; then
    echo "WARNING: Executable not found at $EXECUTABLE"
    exit 0
fi

echo "Updating rpath references in $EXECUTABLE..."
for dylib in "${dylibs[@]}"; do
    filename=$(basename "$dylib")

    # Extract current path
    old_path=$(otool -L "$EXECUTABLE" | awk -v lib="$filename" '$0 ~ lib {print $1; exit}')

    if [ -n "$old_path" ]; then
        echo "Updating: $old_path -> @rpath/$filename"

        # Update reference
        if ! install_name_tool -change "$old_path" "@rpath/$filename" "$EXECUTABLE"; then
            echo "ERROR: Failed to update reference for $filename"
            exit 1
        fi
    fi
done

# Add required RPATH
REQUIRED_RPATH="@loader_path/../Frameworks"
echo "Setting RPATH to $REQUIRED_RPATH..."
if ! install_name_tool -add_rpath "$REQUIRED_RPATH" "$EXECUTABLE"; then
    echo "ERROR: Failed to add RPATH $REQUIRED_RPATH"
    exit 1
fi

# Remove unwanted CMake build paths
UNWANTED_PATTERNS=(
    "_deps/wxwidgets-build/lib"
    "CryptoToysPP/cmake-build-debug"
    "CryptoToysPP/cmake-build-release"
    "CryptoToysPP/build"
    "CryptoToysPP/release"
    # Add more patterns as needed
)

echo "Removing unwanted CMake build RPATHs..."
current_rpaths=$(otool -l "$EXECUTABLE" | grep -A2 LC_RPATH | grep "path " | awk '{print $2}')

for rpath in $current_rpaths; do
    for pattern in "${UNWANTED_PATTERNS[@]}"; do
        if [[ $rpath == *"$pattern"* ]]; then
            echo "Removing CMake RPATH: $rpath"
            if ! install_name_tool -delete_rpath "$rpath" "$EXECUTABLE"; then
                echo "WARNING: Failed to remove RPATH $rpath"
            fi
            break
        fi
    done
done

# Verify final RPATH settings
echo "Verifying final RPATH settings..."
if ! otool -l "$EXECUTABLE" | grep -A2 LC_RPATH; then
    echo "WARNING: Failed to verify RPATH settings"
fi

echo "Operation completed successfully"