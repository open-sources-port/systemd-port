#!/bin/bash

command="$1"
outputLogFile=output.log

if [[ "$OSTYPE" == "darwin"* ]]; then
  echo "Detected macOS"
  # brew install coreutils libgcrypt libxcrypt ccrypt gettext libmount
  # brew meson install python3
  # sudo ln -s /opt/homebrew/bin/grealpath /usr/local/bin/realpath

  if [[ ! -d .venv ]]; then
      python3 -m venv .venv
  fi
  source .venv/bin/activate
  # python3 -m pip install jinja2
  # pip3 install 'jinja2-cli[env]'

  if [[ "$command" == "config"* ]]; then
      echo "Running set up command..."
      export CPPFLAGS="-I/opt/homebrew/include"
      export LDFLAGS="-L/opt/homebrew/lib"
      export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig"
      CC=clang CXX=clang++ meson setup build --buildtype debug --prefix /opt/homebrew --reconfigure --wipe > "${outputLogFile}" 2>&1
      cat "${outputLogFile}"
      echo "Please check details in ${outputLogFile}!"
  elif [[ "$command" == "build"* ]]; then
      echo "Running build command..."
      export CPPFLAGS="-I/opt/homebrew/include"
      export LDFLAGS="-L/opt/homebrew/lib"
      export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig"
      ninja -C build > "${outputLogFile}" 2>&1
      cat "${outputLogFile}"
      echo "Please check details in ${outputLogFile}!"
  else
      echo "Unknown command. Please input 'config' or 'build' as first parameters!"
  fi
else
  echo "Unsupported OS"
  exit 1
fi
