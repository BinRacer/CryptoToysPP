#!/bin/bash

# Strict error handling
set -euo pipefail

# Validate input parameter
if [ $# -eq 0 ]; then
  echo "ERROR: Missing build directory path"
  echo "Usage: $0 /path/to/build/directory"
  exit 1
fi

BUILD_DIR="$1"
LIB_DIR="$BUILD_DIR/lib"
EXECUTABLE="$BUILD_DIR/CryptoToysPP"

# Validate build directory structure
if [ ! -d "$BUILD_DIR" ]; then
  echo "ERROR: Build directory not found at $BUILD_DIR"
  exit 1
fi

# Create lib directory
echo "Creating relative path lib directory: $LIB_DIR"
mkdir -p "$LIB_DIR"

# Find all shared libraries and symlinks
echo "Searching for dynamic library files..."
lib_files=()
while IFS= read -r -d $'\0' file; do
  if file -b "$file" | grep -q "shared object"; then
    lib_files+=("$file")
  fi
done < <(find "$BUILD_DIR" -maxdepth 1 \( -name "*.so*" -o -type l \) -print0)

# Handle symlinks before moving files
echo "Processing symlinks..."
symlinks=($(find "$BUILD_DIR" -maxdepth 1 -type l -name "*.so*"))
for sym in "${symlinks[@]}"; do
  target=$(readlink -f "$sym")
  filename=$(basename "$sym")

  # Convert absolute symlinks to relative
  if [[ "$target" == /* ]]; then
    # Calculate relative path between symlink and target
    rel_path=$(realpath --relative-to="$(dirname "$sym")" "$target")
    ln -sf "$rel_path" "$sym"
    echo "Converted symlink $filename to relative path: $rel_path"
  fi

  # Handle symlink target if in build directory
  if [[ "$target" == "$BUILD_DIR"* ]]; then
    target_name=$(basename "$target")
    if [ ! -f "$LIB_DIR/$target_name" ]; then
      cp "$target" "$LIB_DIR"
      echo "Copied symlink target $target_name to lib directory"
    fi
  fi
done

# Move library files to lib directory
echo "Found ${#lib_files[@]} library files"
echo "Moving to lib directory..."
for file in "${lib_files[@]}"; do
  filename=$(basename "$file")

  # Only process regular files (skip symlinks already handled)
  if [ -f "$file" ] && [[ -L "$file" || "$filename" =~ ^lib.*\.so ]]; then
    echo "Moving $filename"
    mv -f "$file" "$LIB_DIR/"

    # Verify move operation
    if [ ! -f "$LIB_DIR/$filename" ]; then
      echo "ERROR: Verification failed for $filename"
      exit 1
    fi
  fi
done

# Validate executable exists
if [ ! -f "$EXECUTABLE" ]; then
  echo "ERROR: Executable not found at $EXECUTABLE"
  exit 1
fi

# Update RPATH for executable
echo "Setting executable RPATH to relative path..."
patchelf --remove-rpath "$EXECUTABLE" 2>/dev/null || true
patchelf --set-rpath '$ORIGIN/lib' "$EXECUTABLE"
rpath=$(patchelf --print-rpath "$EXECUTABLE")
echo "RPATH set to: $rpath"

# Update library references
echo "Updating library references to relative references..."
needed_libs=$(patchelf --print-needed "$EXECUTABLE")
for lib in $needed_libs; do
  # Only process library names, skip ld-linux
  if [[ "$lib" == lib* ]]; then
    echo "Updated reference: $lib → $lib"
    patchelf --replace-needed "$lib" "$lib" "$EXECUTABLE" 2>/dev/null || true
  fi
done

# Fix libraries in lib directory
echo "Fixing relative references in library files..."
for lib in "$LIB_DIR"/*; do
  if [ -f "$lib" ] && ! [[ -L "$lib" ]]; then
    # Remove any existing RPATH
    patchelf --remove-rpath "$lib" 2>/dev/null || true

    # Set relative RPATH for inter-library dependencies
    patchelf --set-rpath '$ORIGIN' "$lib"

    # Update library's own dependencies
    needed=$(patchelf --print-needed "$lib" 2>/dev/null || true)
    for dep in $needed; do
      patchelf --replace-needed "$dep" "$dep" "$lib" 2>/dev/null || true
    done
  fi
done

# Final verification
echo "Final verification:"
echo "Executable RPATH:"
patchelf --print-rpath "$EXECUTABLE" || true
echo "Executable dependencies:"
patchelf --print-needed "$EXECUTABLE" || true

echo "Operation completed successfully"
exit 0
