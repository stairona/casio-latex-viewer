#!/bin/bash
# docker-build.sh — Build LaTeX_Viewer.g3a via Docker
#
# Builds the app inside Docker (SH4 cross-compile) and exports
# the artifact to dist/LaTeX_Viewer.g3a on the host.
#
# Usage:
#   ./docker-build.sh              # Full build
#   ./docker-build.sh --no-cache   # Force rebuild from scratch

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DIST_DIR="$SCRIPT_DIR/dist"
NO_CACHE=""

if [[ "$1" == "--no-cache" ]]; then
    NO_CACHE="--no-cache"
    echo "Mode: --no-cache (full rebuild)"
fi

echo "============================================================"
echo "LaTeX Viewer — Docker Build"
echo "Project : $SCRIPT_DIR"
echo "Output  : $DIST_DIR/LaTeX_Viewer.g3a"
echo "============================================================"
echo ""

# Verify Docker is running
if ! docker info > /dev/null 2>&1; then
    echo "ERROR: Docker is not running. Start Docker Desktop and try again."
    exit 1
fi

# Verify SSD is mounted (where Docker stores its data)
if [ ! -d "/Volumes/SSD/docker-data" ]; then
    echo "WARNING: /Volumes/SSD/docker-data not found."
    echo "Docker may write to internal storage. Plug in the SSD and check"
    echo "Docker Desktop > Settings > Resources > Disk image location."
    echo ""
    read -p "Continue anyway? (y/N): " CONT
    [[ "$CONT" != "y" && "$CONT" != "Y" ]] && exit 1
fi

# Create dist/ directory
mkdir -p "$DIST_DIR"

echo "[1/2] Building Docker image (this takes 45-60 min first time)..."
echo "      Stage A: cross-compiler | Stage B: app | Stage C: export"
echo ""

cd "$SCRIPT_DIR"
docker build $NO_CACHE --target export --output "$DIST_DIR" .

echo ""
echo "[2/2] Validating artifact..."

ARTIFACT="$DIST_DIR/LaTeX_Viewer.g3a"

if [ ! -f "$ARTIFACT" ]; then
    echo "ERROR: Artifact not found at $ARTIFACT"
    exit 1
fi

if [ ! -s "$ARTIFACT" ]; then
    echo "ERROR: Artifact is empty."
    exit 1
fi

MAGIC=$(xxd -p -l 4 "$ARTIFACT" 2>/dev/null || hexdump -n 4 -e '4/1 "%02x"' "$ARTIFACT")
if [[ "$MAGIC" != "7f454c46" ]]; then
    echo "ERROR: Artifact is not a valid ELF binary (magic: $MAGIC)"
    exit 1
fi

SIZE=$(ls -lh "$ARTIFACT" | awk '{print $5}')
echo "  ✓ $ARTIFACT ($SIZE, ELF valid)"
echo ""
echo "============================================================"
echo "Build complete!"
echo ""
echo "To deploy to calculator, run:"
echo "  ./BUILD_AND_DEPLOY.sh"
echo "============================================================"
