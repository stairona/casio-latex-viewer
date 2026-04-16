#!/bin/bash
# Phase 1: macOS Toolchain Setup for CG50
# Builds sh-elf-gcc cross-compiler using crosstool-NG

set -e  # Exit on error

echo "=========================================="
echo "Phase 1: macOS CG50 Toolchain Setup"
echo "=========================================="
echo ""

# Step 1: Verify Homebrew dependencies
echo "[1/5] Checking Homebrew dependencies..."
brew install pkg-config libpng libusb cmake ninja 2>&1 | grep -E "already|Installing" | head -5

# Step 2: Install crosstool-NG
echo "[2/5] Installing crosstool-NG..."
brew install crosstool-ng 2>&1 | grep -E "already|Installing" | tail -2

# Step 3: Create build directory
echo "[3/5] Setting up build directory..."
TOOLCHAIN_DIR="$HOME/casio-toolchain"
mkdir -p "$TOOLCHAIN_DIR"
cd "$TOOLCHAIN_DIR"
echo "Build directory: $TOOLCHAIN_DIR"

# Step 4: Configure and build SH4 cross-compiler
echo "[4/5] Building sh-elf-gcc cross-compiler (this takes ~45 min)..."
ct-ng sh4 2>&1 | tail -3

# Configure for macOS and set good defaults
sed -i.bak 's/^CT_ALLOW_BUILD_AS_ROOT=.*/CT_ALLOW_BUILD_AS_ROOT=y/' .config
sed -i.bak 's/^CT_ALLOW_BUILD_AS_ROOT_SURE=.*/CT_ALLOW_BUILD_AS_ROOT_SURE=y/' .config

echo "Starting build... (this is long, be patient)"
ct-ng build 2>&1 | tail -20 &
BUILD_PID=$!

# Show progress
while kill -0 $BUILD_PID 2>/dev/null; do
    sleep 30
    echo "  [building... $(date)]"
done
wait $BUILD_PID

# Step 5: Add to PATH
echo "[5/5] Configuring PATH..."
export PATH="$TOOLCHAIN_DIR/x-tools/sh4-unknown-elf/bin:$PATH"

# Update shell config
if [[ ! $(grep "casio-toolchain" ~/.zshrc) ]]; then
    echo 'export PATH="$HOME/casio-toolchain/x-tools/sh4-unknown-elf/bin:$PATH"' >> ~/.zshrc
    echo "Added to ~/.zshrc"
fi

source ~/.zshrc

echo ""
echo "=========================================="
echo "✓ Toolchain setup complete!"
echo "=========================================="
echo ""
echo "Verifying installation..."
sh-elf-gcc --version | head -1
sh-elf-g++ --version | head -1
sh-elf-objcopy --version | head -1
echo ""
echo "Next steps:"
echo "  1. Source your shell: source ~/.zshrc"
echo "  2. Proceed to Phase 2: Build the G3A"
echo ""
