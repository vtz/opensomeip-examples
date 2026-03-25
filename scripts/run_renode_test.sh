#!/bin/bash
################################################################################
# Copyright (c) 2025 Vinicius Tadeu Zein
#
# SPDX-License-Identifier: Apache-2.0
################################################################################
#
# Build and run a Zephyr example on the S32K388 Renode simulator.
#
# Usage:
#   ./scripts/run_renode_test.sh e2e_protection
#   ./scripts/run_renode_test.sh hello_world_server
#   ./scripts/run_renode_test.sh all

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
ZEPHYR_DIR="$PROJECT_DIR/zephyr"
BUILD_BASE="$PROJECT_DIR/build/zephyr"
RENODE_SCRIPT="$ZEPHYR_DIR/renode/s32k388_examples.resc"
BOARD="s32k388_renode"
RENODE_TIMEOUT="${RENODE_TIMEOUT:-15}"

APPS=(
    e2e_protection
    hello_world_server
    hello_world_client
    method_calls_server
    method_calls_client
    events_publisher
    events_subscriber
)

usage() {
    echo "Usage: $0 <app|all>"
    echo ""
    echo "Apps:"
    for app in "${APPS[@]}"; do
        echo "  $app"
    done
    echo "  all  - Run all examples on Renode"
    echo ""
    echo "Environment:"
    echo "  RENODE_TIMEOUT  Simulation timeout in seconds (default: 15)"
    exit 1
}

if [ $# -lt 1 ]; then
    usage
fi

if [ -z "${ZEPHYR_BASE:-}" ]; then
    echo "ERROR: ZEPHYR_BASE not set."
    exit 1
fi

if [ -z "${OPENSOMEIP_ROOT:-}" ]; then
    echo "ERROR: OPENSOMEIP_ROOT not set."
    exit 1
fi

run_app() {
    local app_name="$1"
    local build_dir="$BUILD_BASE/${BOARD}_${app_name}"
    local elf_path="$build_dir/zephyr/zephyr.elf"

    echo ""
    echo "=== Renode Test: $app_name ==="

    # Build if needed
    if [ ! -f "$elf_path" ]; then
        echo "  Building for $BOARD..."
        "$SCRIPT_DIR/zephyr_build.sh" s32k388_renode "$app_name"
    fi

    if [ ! -f "$elf_path" ]; then
        echo "  ERROR: ELF not found: $elf_path"
        return 1
    fi

    echo "  Starting Renode (timeout ${RENODE_TIMEOUT}s)..."
    renode --disable-xwt \
        -e "\$firmware=@$elf_path; i @$RENODE_SCRIPT; start; sleep $RENODE_TIMEOUT; quit" \
        2>&1

    echo "  === $app_name complete ==="
}

APP="$1"

if [ "$APP" = "all" ]; then
    PASSED=0
    FAILED=0
    for app in "${APPS[@]}"; do
        if run_app "$app"; then
            PASSED=$((PASSED + 1))
        else
            FAILED=$((FAILED + 1))
        fi
    done
    echo ""
    echo "=== Renode Test Summary ==="
    echo "  Total:  ${#APPS[@]}"
    echo "  Passed: $PASSED"
    echo "  Failed: $FAILED"
    exit $FAILED
else
    run_app "$APP"
fi
