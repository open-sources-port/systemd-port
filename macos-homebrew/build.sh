#!/bin/bash

command="$1"

if [[ "$OSTYPE" == "darwin"* ]]; then
  echo "Detected macOS"
  # brew install coreutils libgcrypt libxcrypt ccrypt
  # brew meson install python3
  # python3 -m pip install jinja2
  # sudo ln -s /opt/homebrew/bin/grealpath /usr/local/bin/realpath

  if [[ ! -d .venv ]]; then
      python3 -m venv .venv
  fi
  source .venv/bin/activate
  # python3 -m pip install jinja2

  if [[ "$command" == "config"* ]]; then
      echo "Running set up command..."
      CC=clang CXX=clang++ meson setup build --buildtype debug --prefix /opt/homebrew --reconfigure --wipe
  elif [[ "$command" == "build"* ]]; then
      echo "Running build command..."
      ninja -C build
  fi
else
  echo "Unsupported OS"
  exit 1
fi
