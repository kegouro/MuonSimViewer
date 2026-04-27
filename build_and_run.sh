#!/bin/bash
# ============================================================
#  MuonSimViewer v4.0 — Build & Run Script
#  CCTVal / KIT Muon Transport Simulation Analyzer
# ============================================================
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo ""
echo "  ╔══════════════════════════════════════════════════╗"
echo "  ║   MuonSimViewer v4.0  —  CCTVal / KIT           ║"
echo "  ║   C++/Qt6 Muon Simulation Analyzer              ║"
echo "  ╚══════════════════════════════════════════════════╝"
echo ""

# ── Check dependencies ──────────────────────────────────────
check_dep() {
    if ! dpkg -l "$1" &>/dev/null; then
        echo "  [INSTALL] $1"
        return 1
    fi
    return 0
}

NEED_INSTALL=false
for pkg in qt6-base-dev qt6-charts-dev qt6-svg-dev cmake build-essential \
           libgl1-mesa-dev ffmpeg; do
    if ! dpkg -l "$pkg" &>/dev/null 2>&1; then
        NEED_INSTALL=true
        break
    fi
done

if [ "$NEED_INSTALL" = true ]; then
    echo "  Installing dependencies..."
    sudo apt-get install -y \
        qt6-base-dev qt6-base-dev-tools qt6-charts-dev qt6-svg-dev \
        cmake build-essential libgl1-mesa-dev libglu1-mesa-dev ffmpeg \
        2>&1 | grep -E "Setting up|E:" | head -20
fi

# ── Build ───────────────────────────────────────────────────
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

if [ ! -f "Makefile" ] || [ "$1" = "--rebuild" ]; then
    echo "  Configuring..."
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

echo "  Building ($(nproc) cores)..."
make -j$(nproc)

echo ""
echo "  ✅  Build successful: $BUILD_DIR/MuonSimViewer"
echo ""

# ── Launch ──────────────────────────────────────────────────
if [ "$1" != "--build-only" ]; then
    CSV="$2"
    if [ -z "$CSV" ]; then
        # Auto-detect CSV in parent dir
        CSV=$(find "$SCRIPT_DIR" -name "*.csv" | head -1)
    fi

    if [ -n "$CSV" ]; then
        echo "  Launching with: $CSV"
        exec "$BUILD_DIR/MuonSimViewer" "$CSV"
    else
        echo "  Usage: $0 [--rebuild] [path/to/comsol_data.csv]"
        echo "  Launching without data file..."
        exec "$BUILD_DIR/MuonSimViewer"
    fi
fi
