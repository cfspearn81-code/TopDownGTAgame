#!/usr/bin/env bash
# Feature checks on the actual Open City core and Linux SDL client. Both
# launchers are built unless DC_QA_BUILT=1. An optional argument gives the
# archived All-Area 3D source root for exact helm-state parity.
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
tmp="$(mktemp -d)"
trap 'test -z "$tmp" || find "$tmp" -delete' EXIT
export ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
cc=${CC:-gcc}
if [[ ${DC_QA_BUILT:-0} != 1 ]]; then bash scripts/build-linux.sh; bash scripts/build.sh; fi
compile=("$cc" -std=c11 -O0 -g -fsanitize=address,undefined,float-cast-overflow -fno-pie -no-pie)
"${compile[@]}" src/open_city_test.c -o build/open_city_test
build/open_city_test
"${compile[@]}" src/chase_nature_test.c -o build/open_city_nature_contact
build/open_city_nature_contact
"${compile[@]}" scripts/qa/contact_shade_combined.c -o build/open_city_shade_contact
build/open_city_shade_contact
"${compile[@]}" -I src scripts/qa/avatar_wardrobe_contact_test.c -o build/open_city_avatar_contact
build/open_city_avatar_contact
"${compile[@]}" src/wardrobe_test.c -o build/open_city_wardrobe
build/open_city_wardrobe
"${compile[@]}" -DDC_LINUX_NATIVE src/shade_gear_test.c src/linux_save.c -o build/open_city_shade
XDG_DATA_HOME="$tmp/gear" build/open_city_shade
"${compile[@]}" -DDC_LINUX_NATIVE scripts/qa/open_city_save_layers.c src/linux_save.c -o build/open_city_save_layers
XDG_DATA_HOME="$tmp/layers" build/open_city_save_layers
"${compile[@]}" scripts/qa/afterimage_art_test.c -o build/open_city_afterimage
build/open_city_afterimage
"${compile[@]}" src/harbor_boats_test.c -o build/open_city_helm
build/open_city_helm
"$cc" -std=c11 -O2 -fPIC -shared scripts/qa/linux_replay.c -ldl -o build/open_city_sdl.so
python3 scripts/qa/linux_wardrobe.py build/DeadChannel build/open_city_sdl.so build/open_city_wardrobe
python3 scripts/qa/shade_gear_host.py build/DeadChannel build/open_city_sdl.so build/open_city_shade
python3 scripts/qa/afterimage_contact_host.py build/DeadChannel
python3 scripts/qa/harbor_boats_host.py --3d build/DeadChannel build/open_city_sdl.so build/open_city_helm "$tmp/helm"
if [[ $# -eq 1 ]]; then
 "$cc" -std=c11 -O2 -I "$1/src" scripts/qa/boat_motion_parity.c -o "$tmp/old-helm"
 "$cc" -std=c11 -O2 -I src scripts/qa/boat_motion_parity.c -o "$tmp/new-helm"
 "$tmp/old-helm" > "$tmp/old-helm.trace"
 "$tmp/new-helm" > "$tmp/new-helm.trace"
 cmp "$tmp/old-helm.trace" "$tmp/new-helm.trace"
 echo "Helm/traffic bit-for-bit state parity: $(wc -l < "$tmp/new-helm.trace") frames."
fi
