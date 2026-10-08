#!/bin/sh
# Runs kas in its container. Fetched layers and all build output go under yocto/,
# which is git-ignored, so `git clean -fdx` removes everything kas creates.
#
# Usage: scripts/kas.sh <kas command> [args]
#   e.g. scripts/kas.sh build kas/pivision-rpi3.yml
set -eu

REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
export KAS_WORK_DIR="$REPO_ROOT/yocto"
mkdir -p "$KAS_WORK_DIR"
cd "$REPO_ROOT"

exec kas-container "$@"

