#!/bin/bash
set -e  # Stop on first error

echo "=========================="
echo "1. Exiting any ROOT session..."
echo "=========================="
# Make sure no ROOT session is running
pkill -f root || true

echo "=========================="
echo "2. Removing old build directories and shared libraries"
echo "=========================="
rm -rf /adaqfs/home/hamoller/jonesdc/macros/HelPat_t/build
rm -rf /adaqfs/home/hamoller/jonesdc/macros/HelPat_t/*.so
rm -rf /adaqfs/home/hamoller/jonesdc/macros/HelPat_t/*.rootmap
rm -rf /adaqfs/home/hamoller/jonesdc/macros/GoodHelicities/build
rm -rf /adaqfs/home/hamoller/jonesdc/macros/GoodHelicities/*.so
rm -rf /adaqfs/home/hamoller/jonesdc/macros/GoodHelicities/*.rootmap

echo "=========================="
echo "3. Rebuilding HelPat_t library"
echo "=========================="
cd /adaqfs/home/hamoller/jonesdc/macros/HelPat_t
mkdir -p build && cd build
cmake ..
make -j$(nproc)

echo "=========================="
echo "4. Rebuilding GoodHelicities library"
echo "=========================="
cd /adaqfs/home/hamoller/jonesdc/macros/GoodHelicities
mkdir -p build && cd build
cmake ..
make -j$(nproc)

echo "=========================="
echo "5. Rebuild complete. Start a fresh ROOT session!"
echo "=========================="
echo "In ROOT:"
echo ".L /adaqfs/home/hamoller/jonesdc/macros/printGoodHelicities.C+"
