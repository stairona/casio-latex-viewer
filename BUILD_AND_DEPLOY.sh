#!/bin/bash
# BUILD_AND_DEPLOY.sh — LaTeX Viewer for Casio fx-CG50
#
# Usage:
#   ./BUILD_AND_DEPLOY.sh                  # Full interactive build + deploy
#   NO_PROMPTS=1 ./BUILD_AND_DEPLOY.sh     # Skip all interactive prompts (CI/Docker)
#   SKIP_DEPLOY=1 ./BUILD_AND_DEPLOY.sh    # Build only, do not copy to calculator
#
# Primary build path: Docker + SSD (see DOCKER_PLAN.md)
# This script is used for the host-side deploy step (Phase 4 of DOCKER_PLAN.md).
# The cross-compiler and G3A artifact are produced by Docker.

set -e

# ── Resolve project root from script location (no hardcoded paths) ──────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"
DIST_DIR="$PROJECT_DIR/dist"

# ── Flags ────────────────────────────────────────────────────────────────────
NO_PROMPTS="${NO_PROMPTS:-0}"
SKIP_DEPLOY="${SKIP_DEPLOY:-0}"

prompt() {
    if [ "$NO_PROMPTS" = "1" ]; then
        echo "[NO_PROMPTS] Skipping prompt: $1"
    else
        read -p "$1"
    fi
}

echo "============================================================"
echo "LaTeX Viewer — Build & Deploy"
echo "PROJECT_DIR : $PROJECT_DIR"
echo "DIST_DIR    : $DIST_DIR"
echo "NO_PROMPTS  : $NO_PROMPTS"
echo "SKIP_DEPLOY : $SKIP_DEPLOY"
echo "============================================================"
echo ""

# ── Artifact validation ──────────────────────────────────────────────────────
echo "[1/3] Validating build artifact..."

ARTIFACT="$DIST_DIR/LaTeX_Viewer.g3a"

if [ ! -f "$ARTIFACT" ]; then
    echo ""
    echo "ERROR: Artifact not found at $ARTIFACT"
    echo ""
    echo "The G3A file is produced by the Docker build (DOCKER_PLAN.md Phase 4)."
    echo "Run the Docker build first, then re-run this script."
    exit 1
fi

if [ ! -s "$ARTIFACT" ]; then
    echo "ERROR: Artifact exists but is empty: $ARTIFACT"
    exit 1
fi

# ELF magic number check (first 4 bytes: 7f 45 4c 46)
MAGIC=$(xxd -p -l 4 "$ARTIFACT" 2>/dev/null || hexdump -n 4 -e '4/1 "%02x"' "$ARTIFACT" 2>/dev/null || echo "unknown")
if [[ "$MAGIC" != "7f454c46" ]]; then
    echo "ERROR: Artifact does not appear to be a valid ELF binary (magic: $MAGIC)"
    echo "Expected: 7f454c46 (ELF)"
    exit 1
fi

SIZE=$(ls -lh "$ARTIFACT" | awk '{print $5}')
echo "  ✓ Artifact valid: $ARTIFACT ($SIZE)"
echo ""

# ── Skip deploy if requested ─────────────────────────────────────────────────
if [ "$SKIP_DEPLOY" = "1" ]; then
    echo "[SKIP_DEPLOY] Build validation passed. Skipping calculator deployment."
    echo ""
    echo "Artifact: $ARTIFACT"
    exit 0
fi

# ── Deploy to calculator ─────────────────────────────────────────────────────
echo "[2/3] Deploying to calculator..."
echo ""
echo "Connect your Casio fx-CG50 via USB, then on the calculator:"
echo "  HOME → [System] → [USB] → enable USB mode"
echo ""

prompt "Press ENTER when calculator is connected and ready (Ctrl+C to cancel)..."
echo ""

# Detect calculator volume
CASIO_MOUNT=$(ls /Volumes/ 2>/dev/null | grep -i -E "^CG|casio" | head -1 || echo "")

if [ -z "$CASIO_MOUNT" ]; then
    echo "  No Casio device auto-detected. Available volumes:"
    ls /Volumes/
    echo ""
    if [ "$NO_PROMPTS" = "1" ]; then
        echo "ERROR: Calculator not found and NO_PROMPTS=1. Cannot deploy."
        exit 1
    fi
    read -p "Enter calculator volume name (e.g. CG50), or 'CANCEL' to skip: " CASIO_MOUNT
    [ "$CASIO_MOUNT" = "CANCEL" ] && exit 0
fi

CASIO_PATH="/Volumes/$CASIO_MOUNT"
if [ ! -d "$CASIO_PATH" ]; then
    echo "ERROR: Path not found: $CASIO_PATH"
    exit 1
fi

echo "  Calculator found at: $CASIO_PATH"
mkdir -p "$CASIO_PATH/addin/"
cp "$ARTIFACT" "$CASIO_PATH/addin/"
echo "  ✓ Copied to: $CASIO_PATH/addin/LaTeX_Viewer.g3a"

echo ""
echo "[3/3] Ejecting calculator..."
diskutil unmount "$CASIO_PATH"
echo "  ✓ Safely ejected"
echo ""

# ── Verify on device ─────────────────────────────────────────────────────────
echo "============================================================"
echo "On your calculator:"
echo "  HOME → [System] → [Advanced] → [Program List]"
echo "  Look for 'LaTeX Viewer' and press EXE to run"
echo "============================================================"
echo ""

if [ "$NO_PROMPTS" != "1" ]; then
    read -p "Did LaTeX Viewer appear on the calculator? (y/n): " VERIFY
    if [ "$VERIFY" = "y" ] || [ "$VERIFY" = "Y" ]; then
        echo ""
        echo "SUCCESS — LaTeX Viewer is installed and running!"
    else
        echo ""
        echo "Troubleshooting:"
        echo "  • Verify file is in /addin/ folder (not root)"
        echo "  • Check: ls $CASIO_PATH/addin/"
        echo "  • Try restarting the calculator"
        echo "  • Verify binary: file $ARTIFACT"
    fi
fi

echo ""
echo "Done."
