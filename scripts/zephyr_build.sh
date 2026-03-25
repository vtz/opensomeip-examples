#!/bin/bash
################################################################################
# Copyright (c) 2025 Vinicius Tadeu Zein
#
# SPDX-License-Identifier: Apache-2.0
################################################################################
#
# Build helper for Zephyr example targets.
#
# Usage:
#   ./scripts/zephyr_build.sh native_sim hello_world_server
#   ./scripts/zephyr_build.sh s32k388_renode e2e_protection
#   ./scripts/zephyr_build.sh native_sim all
#   ./scripts/zephyr_build.sh clean

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
ZEPHYR_DIR="$PROJECT_DIR/zephyr"
BUILD_BASE="$PROJECT_DIR/build/zephyr"

APPS=(
    hello_world_server
    hello_world_client
    method_calls_server
    method_calls_client
    events_publisher
    events_subscriber
    e2e_protection
)

usage() {
    echo "Usage: $0 <target> [app|all]"
    echo ""
    echo "Targets:"
    echo "  native_sim      - Build for native_sim (host simulation)"
    echo "  s32k388_renode   - Build for S32K388 Renode simulation"
    echo "  clean            - Remove Zephyr build artifacts"
    echo ""
    echo "Apps:"
    for app in "${APPS[@]}"; do
        echo "  $app"
    done
    echo "  all              - Build all examples"
    exit 1
}

if [ $# -lt 1 ]; then
    usage
fi

TARGET="$1"
APP="${2:-}"

if [ "$TARGET" = "clean" ]; then
    echo "Cleaning Zephyr build artifacts..."
    rm -rf "$BUILD_BASE"
    echo "Done."
    exit 0
fi

if [ -z "${ZEPHYR_BASE:-}" ]; then
    echo "ERROR: ZEPHYR_BASE not set. Source the Zephyr environment first:"
    echo "  source <zephyr-workspace>/zephyr/zephyr-env.sh"
    echo "Or run inside the Docker container:"
    echo "  docker compose -f docker/docker-compose.zephyr.yml run zephyr-dev bash"
    exit 1
fi

if [ -z "${OPENSOMEIP_ROOT:-}" ]; then
    echo "ERROR: OPENSOMEIP_ROOT not set. Point it to the opensomeip source tree:"
    echo "  export OPENSOMEIP_ROOT=/path/to/opensomeip"
    exit 1
fi

case "$TARGET" in
    native_sim)      BOARD="native_sim" ;;
    s32k388_renode)  BOARD="s32k388_renode" ;;
    *)
        echo "ERROR: Unknown target '$TARGET'"
        usage
        ;;
esac

build_app() {
    local app_name="$1"
    local app_dir="$ZEPHYR_DIR/$app_name"
    local build_dir="$BUILD_BASE/${BOARD}_${app_name}"

    if [ ! -d "$app_dir" ]; then
        echo "ERROR: App directory not found: $app_dir"
        return 1
    fi

    echo ""
    echo "=== Building $app_name for $BOARD ==="

    local extra_args=()
    if [ "$BOARD" = "s32k388_renode" ]; then
        extra_args+=("-DBOARD_ROOT=$OPENSOMEIP_ROOT/zephyr")
    fi

    west build -b "$BOARD" "$app_dir" -d "$build_dir" --pristine auto -- \
        -DOPENSOMEIP_ROOT="$OPENSOMEIP_ROOT" "${extra_args[@]}" 2>&1

    echo "  Build: OK ($build_dir)"
}

if [ "$APP" = "all" ] || [ -z "$APP" ]; then
    echo "=== Building all examples for $BOARD ==="
    FAILED=0
    for app in "${APPS[@]}"; do
        if ! build_app "$app"; then
            FAILED=$((FAILED + 1))
        fi
    done
    echo ""
    echo "=== Build Summary ==="
    echo "  Total:  ${#APPS[@]}"
    echo "  Failed: $FAILED"
    exit $FAILED
else
    build_app "$APP"
fi
