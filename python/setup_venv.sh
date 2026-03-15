#!/usr/bin/env bash
# Creates a Python virtual environment and installs dependencies.
# Run from the repository root:
#   bash python/setup_venv.sh

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VENV_DIR="$REPO_ROOT/.venv"

if [ -d "$VENV_DIR" ]; then
    echo "Virtual environment already exists at $VENV_DIR"
else
    echo "Creating virtual environment at $VENV_DIR ..."
    python3 -m venv "$VENV_DIR"
fi

echo "Installing dependencies ..."
"$VENV_DIR/bin/pip" install --upgrade pip
"$VENV_DIR/bin/pip" install -r "$REPO_ROOT/python/requirements.txt"

echo ""
echo "Done. Activate with:"
echo "  source $VENV_DIR/bin/activate"
