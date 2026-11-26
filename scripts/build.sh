#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
flags=(-D_WIN_NATIVE -std=c11 -O2 -mabi=ms -mno-red-zone -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-stack-protector -fno-builtin -fno-ident -fno-tree-loop-distribute-patterns -fshort-wchar -fpie)
for unit in game windows; do
 gcc "${flags[@]}" -c "src/$unit.c" -o "build/$unit.o"
 python3 scripts/coff.py "build/$unit.o" "build/$unit.obj"
done
python3 scripts/coff.py imports build/imports.obj
ld -m i386pep --subsystem windows --entry entry --no-insert-timestamp --high-entropy-va --nxcompat --dynamicbase -s -o build/DeadChannel.exe build/game.obj build/windows.obj build/imports.obj
file build/DeadChannel.exe
