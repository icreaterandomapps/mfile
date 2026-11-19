#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

if ! command -v pkg-config >/dev/null 2>&1; then
  echo "pkg-config is required. Install build dependencies for your distro (see README.md)."
  exit 1
fi

if ! pkg-config --exists gtk+-3.0 gio-2.0; then
  echo "GTK3/GIO development packages not found. Install build dependencies for your distro (see README.md)."
  exit 1
fi

make
echo "Build successful: $(pwd)/mfile"
