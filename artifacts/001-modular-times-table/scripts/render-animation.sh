#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd -- "${script_dir}/../../.." && pwd)"
renderer="${repo_dir}/build/artifacts/001-modular-times-table/modular_times_table"
output="${repo_dir}/out/001-vertical-master.mp4"
width=1080
height=1920
fps=30
duration=8
points=720
start=1
end=6

usage() {
  cat <<EOF
Usage: $0 [options]
  --renderer PATH   Renderer executable
  --output PATH     MP4 destination
  --width PX        Frame width (default: 1080)
  --height PX       Frame height (default: 1920)
  --fps N           Frames per second (default: 30)
  --duration SEC    Duration in whole seconds (default: 8)
  --points N        Number of chords (default: 720)
  --start M         Starting multiplier (default: 1)
  --end M           Ending multiplier (default: 6)
EOF
}

while (($#)); do
  case "$1" in
    --renderer) renderer="$2"; shift 2 ;;
    --output) output="$2"; shift 2 ;;
    --width) width="$2"; shift 2 ;;
    --height) height="$2"; shift 2 ;;
    --fps) fps="$2"; shift 2 ;;
    --duration) duration="$2"; shift 2 ;;
    --points) points="$2"; shift 2 ;;
    --start) start="$2"; shift 2 ;;
    --end) end="$2"; shift 2 ;;
    --help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
  esac
done

for tool in rsvg-convert ffmpeg ffprobe awk; do
  command -v "${tool}" >/dev/null || {
    echo "error: ${tool} is required" >&2
    exit 1
  }
done
[[ -x "${renderer}" ]] || {
  echo "error: renderer not found at ${renderer}; build the project first" >&2
  exit 1
}
for value in width height fps duration points; do
  candidate="${!value}"
  [[ "${candidate}" =~ ^[0-9]+$ ]] && ((candidate > 0)) || {
    echo "error: --${value} must be a positive integer" >&2
    exit 2
  }
done
((width >= 64 && width <= 16384 && height >= 64 && height <= 16384)) || {
  echo "error: dimensions must be from 64 to 16384" >&2
  exit 2
}
((fps <= 120 && duration <= 600 && points >= 3 && points <= 200000)) || {
  echo "error: fps, duration, or points exceeds the supported range" >&2
  exit 2
}

frame_count=$((fps * duration))
work_dir="$(mktemp -d "${TMPDIR:-/tmp}/artifact-001-frames.XXXXXX")"
cleanup() { rm -rf -- "${work_dir}"; }
trap cleanup EXIT
mkdir -p -- "$(dirname -- "${output}")"

for ((frame = 0; frame < frame_count; ++frame)); do
  multiplier="$(awk -v i="${frame}" -v count="${frame_count}" \
    -v first="${start}" -v last="${end}" \
    'BEGIN {
      t = count > 1 ? i / (count - 1) : 0;
      eased = t * t * (3 - 2 * t);
      printf "%.9f", first + (last - first) * eased;
    }')"
  frame_name="$(printf '%s/frame-%06d' "${work_dir}" "${frame}")"
  "${renderer}" \
    --output "${frame_name}.svg" \
    --points "${points}" \
    --multiplier "${multiplier}" \
    --width "${width}" \
    --height "${height}" \
    --radius 0.46 \
    --line-width 0.65 \
    --line-alpha 0.12 >/dev/null
  rsvg-convert \
    --width "${width}" \
    --height "${height}" \
    --output "${frame_name}.png" \
    "${frame_name}.svg"
done

ffmpeg -hide_banner -loglevel error -y \
  -framerate "${fps}" \
  -i "${work_dir}/frame-%06d.png" \
  -an \
  -c:v libx264 \
  -profile:v high \
  -pix_fmt yuv420p \
  -crf 17 \
  -x264-params colorprim=bt709:transfer=bt709:colormatrix=bt709 \
  -movflags +faststart \
  -color_primaries bt709 \
  -color_trc bt709 \
  -colorspace bt709 \
  "${output}"

ffprobe -v error \
  -select_streams v:0 \
  -show_entries stream=codec_name,pix_fmt,width,height,r_frame_rate,color_space,color_transfer,color_primaries \
  -show_entries format=duration \
  -of default=noprint_wrappers=1 \
  "${output}"

echo "Rendered animation master: ${output}"
