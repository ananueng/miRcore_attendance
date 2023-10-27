#!/bin/bash

# attendance tracker
# Anan

# Stop on errors
# See https://vaneyckt.io/posts/safer_bash_scripts_with_set_euxo_pipefail/
set -Eeuo pipefail

usage() {
  echo "Usage: ./run (setup | help)"
}

if [ $# -gt 2 ]; then
  usage
  exit 1
fi

python3 run.py

