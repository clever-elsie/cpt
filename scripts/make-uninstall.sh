#!/usr/bin/env bash
set -euo pipefail

# Inputs via environment (preferred) or defaults
BUILD_DIR=${BUILD_DIR:-build}
INSTALL_PREFIX_FILE="$BUILD_DIR/.installed_prefix"

# Determine prefix
if [[ -n "${PREFIX:-}" ]]; then
  prefix="$PREFIX"
elif [[ -f "$INSTALL_PREFIX_FILE" ]]; then
  prefix="$(cat "$INSTALL_PREFIX_FILE")"
else
  prefix="/usr/local"
fi

target="$prefix/bin/cpt"
if [[ -f "$target" || -L "$target" ]]; then
  echo "Removing $target"
  rm -f "$target"
else
  echo "Not found: $target"
fi


