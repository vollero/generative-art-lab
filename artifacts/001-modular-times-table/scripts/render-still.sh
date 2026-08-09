#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd -- "${script_dir}/../../.." && pwd)"
renderer="${repo_dir}/build/artifacts/001-modular-times-table/modular_times_table"
svg_output="${repo_dir}/out/001-featured.svg"
png_output="${repo_dir}/out/001-featured.png"
size=2160

usage() {
  echo "Usage: $0 [--renderer PATH] [--svg PATH] [--png PATH] [--size PX]"
}

while (($#)); do
  case "$1" in
    --renderer) renderer="$2"; shift 2 ;;
    --svg) svg_output="$2"; shift 2 ;;
    --png) png_output="$2"; shift 2 ;;
    --size) size="$2"; shift 2 ;;
    --help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
  esac
done

command -v rsvg-convert >/dev/null || {
  echo "error: rsvg-convert is required" >&2
  exit 1
}
[[ -x "${renderer}" ]] || {
  echo "error: renderer not found at ${renderer}; build the project first" >&2
  exit 1
}
[[ "${size}" =~ ^[0-9]+$ ]] && ((size >= 64 && size <= 16384)) || {
  echo "error: --size must be an integer from 64 to 16384" >&2
  exit 2
}

mkdir -p -- "$(dirname -- "${svg_output}")" "$(dirname -- "${png_output}")"
"${renderer}" \
  --output "${svg_output}" \
  --points 720 \
  --multiplier 2 \
  --width 2160 \
  --height 2160 \
  --radius 0.46 \
  --line-width 0.65 \
  --line-alpha 0.12

rsvg-convert \
  --width "${size}" \
  --height "${size}" \
  --keep-aspect-ratio \
  --output "${png_output}" \
  "${svg_output}"

echo "Rendered still master: ${svg_output}"
echo "Rendered PNG derivative: ${png_output}"
