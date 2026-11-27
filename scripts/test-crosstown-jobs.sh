#!/usr/bin/env bash
# Isolated engine fixtures. For optional fresh keyboard replays first build
# native hosts (scripts/build.sh, scripts/build-linux.sh), then add --hosts.
set -euo pipefail
cd "$(dirname "$0")/.."
d=$(mktemp -d)
trap 'test -n "$d" && find "$d" -delete' EXIT
for suite in jobs_hour_test fare_soak listing_soak seam_route_test minimap_legend_test rider_booking_test; do
 "${CC:-cc}" -std=c11 -O0 "scripts/qa/$suite.c" -o "$d/$suite"
done
"$d/jobs_hour_test" all
"$d/seam_route_test"
"$d/minimap_legend_test"
"$d/rider_booking_test"
for seed in 10059 24023; do "$d/fare_soak" "$seed"; "$d/listing_soak" "$seed"; done
if [[ ${1-} == --hosts || ${1-} == --all-stops ]]; then
 python3 scripts/qa/outer_taxi_replay.py
 python3 scripts/qa/outer_taxi_replay.py --radio
 if [[ ${1-} == --all-stops ]];then
  for stop in 0 1 2 3 4 5; do python3 scripts/qa/outer_taxi_replay.py --outbound "$stop"; done
 else python3 scripts/qa/outer_taxi_replay.py --outbound;fi
fi
