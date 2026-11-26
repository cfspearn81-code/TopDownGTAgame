#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build/verdict
for m in core taxi driving street media;do
 gcc -std=c11 -O2 src/verdict_${m}_test.c -o build/verdict/$m
done
build/verdict/core
build/verdict/taxi
build/verdict/street
build/verdict/media
for take in film valet valet-near puzzle race parity; do LANE=1038 build/verdict/driving "$take"; done
gcc -std=c11 -O2 -Isrc scripts/qa/verdict_explore_test.c -o build/verdict/explore
build/verdict/explore
gcc -std=c11 -O2 -Isrc scripts/qa/media_tour.c -lm -o build/verdict/media
for t in 4 7 8; do for b in 0 1; do
 d=build/verdict/news-$t-$b;mkdir -p "$d"; args=()
 if [[ $t == 7 ]];then
  if [[ $b == 0 ]];then args=(1037,1278,1 1037,1016,1 861,1016,1);else args=(1037,1278,1 1037,1034,1 1853,1034,1);fi
 fi
 build/verdict/media "$t" "$b" "$d/career.dat" "$d" "${args[@]}" >"$d/actions.txt"
done;done
