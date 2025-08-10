#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR=${BUILD_DIR:-build}
INSTALL_PREFIX_FILE="$BUILD_DIR/.installed_prefix"

if [[ -n "${PREFIX:-}" ]]; then
  prefix="$PREFIX"
elif [[ -f "$INSTALL_PREFIX_FILE" ]]; then
  prefix="$(cat "$INSTALL_PREFIX_FILE")"
else
  prefix="/usr/local"
fi

current_branch="$(git rev-parse --abbrev-ref HEAD 2>/dev/null || true)"
git fetch origin main >/dev/null 2>&1 || { echo "git fetch failed"; exit 1; }
local_main="$(git rev-parse --verify refs/heads/main 2>/dev/null || true)"
remote_main="$(git rev-parse --verify refs/remotes/origin/main 2>/dev/null || true)"
if [[ -z "$remote_main" ]]; then
  echo "origin/main not found"
  exit 1
fi

if [[ -n "$local_main" && "$local_main" == "$remote_main" ]]; then
  echo "Already up to date (main)"
  exit 0
fi

echo "Updating main..."
if [[ "$current_branch" == "main" ]]; then
  git pull --ff-only origin main
  sudo make install PREFIX="$prefix"
else
  if [[ -z "$local_main" ]]; then
    git checkout -q -B main origin/main
  else
    git checkout -q main
    git pull --ff-only origin main
  fi
  sudo make install PREFIX="$prefix"
  git checkout -q "$current_branch"
fi


