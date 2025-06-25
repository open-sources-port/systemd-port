#!/bin/bash

if [[ "$OSTYPE" == "darwin"* ]]; then
  echo "Detected macOS"
  CC=clang CXX=clang++ meson setup build --buildtype debug --prefix /opt/linux-port --reconfigure --wipe
elif [[ "$OS" == "Windows_NT" ]]; then
  echo "Detected Windows"
  CC=cl.exe CXX=cl.exe meson setup build --buildtype debug --prefix C:/linux-port/ --reconfigure --wipe
else
  echo "Unsupported OS"
  exit 1
fi
