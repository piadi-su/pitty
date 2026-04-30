#!/usr/bin/env bash

set -e

REPO_DIR="/tmp/pitty_build"

git clone https://github.com/piadi-su/pitty.git "$REPO_DIR"

cd "$REPO_DIR"
cd scripts

chmod +x install.sh

./install.sh
