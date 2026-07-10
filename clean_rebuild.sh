#!/bin/bash
# rebuild_all.sh
# Fully clean and rebuild HelPat_t and GoodHelicities

set -e  # exit on any error
echo "=== Starting full clean rebuild ==="

# --- Step 0: Set directories ---
PAT_DIR="/adaqfs/home/hamoller/jonesdc/macros/HelPat_t"
GH_DIR="/adaqfs/home/hamoller/jonesdc/macros/GoodHelicities"

# --- Step 1: Kill any ROOT processes using old libraries ---
echo "Killing ROOT/ACLiC processes using old libraries..."
ROOT_PROCS=$(lsof | grep -E "${PAT_DIR}|${GH_DIR}" | awk '{print $2}' | sort -u)
if [ -n "$ROOT_PROCS" ]; then
    echo "Found ROOT processes using old libraries: $ROOT_PROCS"
    kill -9 $ROOT_PROCS
fi

# --- Step 2: Remove old build directories ---
echo "Removing old build directories..."
rm -rf "$PAT_DIR/build"
rm -rf "$GH_DIR/build"

# --- Step 3: Remove old ROOT cache files ---
echo "Removing old ROOT cache files..."
rm -rf ~/.root/*

# --- Step 4: Build HelPat_t ---
echo "Building HelPat_t..."
mkdir -p "$PAT_DIR/build"
cd "$PAT_DIR/build"
cmake ..
make -j$(nproc)

# --- Step 5: Build GoodHelicities ---
echo "Building GoodHelicities..."
mkdir -p "$GH_DIR/build"
cd "$GH_DIR/build"
cmake ..
make -j$(nproc)

# --- Step 6: Print ROOT load commands ---
echo "=== Rebuild complete ==="
echo "In ROOT, load the libraries with:"
echo "gSystem->Load(\"$PAT_DIR/build/libHelPat_t.so\");"
echo "gSystem->Load(\"$GH_DIR/build/libGoodHelicities.so\");"
echo ".L printGoodHelicities.C+"
