#!/bin/bash
# Install MuonSimViewer system-wide
BINARY="$(dirname "$0")/build/MuonSimViewer"
DEST="/usr/local/bin/muonsimviewer"
if [ ! -f "$BINARY" ]; then
    echo "Binary not found. Run ./build_and_run.sh --build-only first."
    exit 1
fi
sudo cp "$BINARY" "$DEST"
echo "Installed to $DEST"
echo "Run: muonsimviewer [path/to/data.csv]"
