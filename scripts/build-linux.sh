#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
[[ $(uname -s) == Linux && $(uname -m) == x86_64 ]] || { echo 'Build on an x86-64 Linux host.' >&2; exit 1; }
mkdir -p build
host_flags=()
linux_libs=(-l:libSDL2-2.0.so.0)
if command -v pkg-config >/dev/null && pkg-config --exists sdl2; then
  read -r -a sdl_cflags <<< "$(pkg-config --cflags sdl2)"
  read -r -a linux_libs <<< "$(pkg-config --libs sdl2)"
  host_flags=(-DDC_USE_SYSTEM_SDL "${sdl_cflags[@]}")
fi
"${CC:-gcc}" -std=c11 -O2 -fPIE -pie -Wl,-z,relro,-z,now -DDC_LINUX_NATIVE "${host_flags[@]}" \
  src/game.c src/linux.c src/linux_save.c "${linux_libs[@]}" -o build/DeadChannel
file build/DeadChannel
