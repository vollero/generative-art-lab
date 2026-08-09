# 001 — Modular Times Table

## What you are seeing

Hundreds of straight chords form a shape that looks curved. Each point on a
circle is connected to another point selected by modular multiplication. The
featured multiplier `2` reveals a cardioid-like envelope.

## Idea

Number the `n` equally spaced points around a circle from `0` to `n - 1`. For
every point `i`, draw a line to:

```text
destination = (multiplier × i) mod n
```

The modulo operation wraps every destination back around the circle. Adjacent
source points also have adjacent destinations until the mapping wraps, so the
many lines share a smooth-looking envelope. The apparent curve is not drawn by
the program: it emerges from the density of straight segments.

The renderer also permits fractional multipliers. A fractional destination is
interpreted as a continuous angle between discrete points, which makes smooth
parameter animation possible while preserving the same construction.

## Algorithm

1. Place point `i` at angle `2πi/n - π/2`.
2. Compute `(multiplier × i) mod n` for its destination index.
3. Convert that possibly fractional index to a second angle.
4. Draw a chord between the two Cartesian coordinates.
5. Repeat for all `n` source points with low opacity so density becomes tone.

Runtime and SVG size are both linear in `point_count`; the renderer uses
constant working memory apart from the output stream.

## Build and run

Requirements: CMake 3.20+ and a C++20 compiler. No graphics dependency is
required for SVG output.

From the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/artifacts/001-modular-times-table/modular_times_table \
  --output out/001-featured.svg \
  --points 720 --multiplier 2 --width 2160 --height 2160 \
  --radius 0.46 --line-width 0.65 --line-alpha 0.12
```

On a multi-config generator such as Visual Studio, the executable may be under
a `Release` subdirectory. Run the test suite with `ctest --test-dir build`.

## Parameters

| Parameter | Type / range | Default | Visible effect | Interactions |
| --- | --- | --- | --- | --- |
| `--points` | integer, 3–200000 | 360 | Adds chords and makes the envelope denser/smoother. | Higher values usually need lower opacity or line width. |
| `--multiplier` | real, ≥ 0 | 2 | Changes the family and number of apparent lobes. | Integers land on points; fractions enable smooth motion. |
| `--width`, `--height` | integer, 64–16384 | 2160 | Set canvas dimensions and aspect ratio. | Radius follows the shorter dimension. |
| `--radius` | real, (0, 0.5] | 0.46 | Scales the circle within the canvas. | At 0.5, lines may touch the edge. |
| `--line-width` | real, > 0 | 0.65 | Changes graphic weight. | Dense drawings need thinner lines. |
| `--line-alpha` | real, 0–1 | 0.16 | Controls accumulation, contrast, and glow-like density. | Balance against point count and line width. |
| `--background` | SVG/CSS color | `#080b14` | Sets the field color. | Contrast with foreground affects perceived density. |
| `--foreground` | SVG/CSS color | `#70e1f5` | Sets every chord color. | Opacity blends it into the background. |
| `--accent` | SVG/CSS color | `#ffd166` | Highlights the selected construction chord. | Used with `--highlight`. |
| `--chords` | integer, 0–point count | all | Reveals the construction progressively. | The highlighted index must already be visible to appear. |
| `--highlight` | integer point index | none | Accents one source-to-destination mapping. | Best combined with low point counts. |
| `--show-points` | flag | off | Adds markers at each discrete position. | Intended for explanatory renders. |
| `--show-labels` | flag; ≤200 points | off | Adds numeric indices around the circle. | Large labels need adequate radius/canvas margin. |
| `--output` | path | `modular-times-table.svg` | Chooses the SVG destination. | Parent directories are created automatically. |

## Presets and reproduction

[`presets/featured.json`](presets/featured.json) records the featured inputs in
a tool-neutral format. The current executable uses explicit command-line
options; this makes every render command self-documenting and avoids a JSON
runtime dependency.

The algorithm is deterministic and uses no randomness, so no seed is required.
Floating-point coordinate formatting is fixed to three decimal places. Outputs
should be stable for the same parameters, though the final rasterization of SVG
may vary slightly between renderers.

## Export and experiments

SVG is the archival master for this line-based piece. Rasterize it with a
color-managed SVG tool at the exact target resolution. For animation, render a
sequence while incrementing `--multiplier`; encode that sequence separately so
the mathematical renderer remains independent of a video codec.

The export helpers require `rsvg-convert`; animation additionally requires
FFmpeg and FFprobe. Build the renderer first, then run:

```bash
# Reproduce the featured SVG and a 2160 px PNG.
./artifacts/001-modular-times-table/scripts/render-still.sh

# Render the 8-second, 1080 × 1920, 30 fps vertical animation master.
./artifacts/001-modular-times-table/scripts/render-animation.sh

```

Both scripts accept options through `--help`. The animation moves from
multiplier `1` to `6` with smoothstep easing, writes H.264 High Profile in an
MP4 container, tags SDR output as BT.709, and moves the MP4 index to the front
for progressive download. It intentionally contains no audio; narration and
licensed sound belong to the editorial timeline, not the mathematical master.

For a quick pipeline test:

```bash
./artifacts/001-modular-times-table/scripts/render-animation.sh \
  --width 360 --height 640 --fps 10 --duration 1 --points 120 \
  --output out/001-pipeline-smoke.mp4
```

Try:

- multipliers from `0` to `12` in increments of `0.01`;
- integer multipliers `2`, `3`, `4`, and `5` at the same point count;
- low point counts to expose the discrete construction;
- a gradual rise in point count followed by a multiplier sweep;
- portrait dimensions such as `2160 × 3840` with centered square geometry.

## Credits and references

This implementation uses the established modular multiplication-circle
construction, also called a residue design or multiplication-circle design.

- David Richeson, “[How Much String to String a Cardioid?](https://arxiv.org/abs/2311.15101),” 2023. Defines the construction `k → ak (mod n)`, identifies its epicycloid envelopes, and analyzes total chord length.
- David Richeson and Susan H. Marshall, “[Curve Stitching and Dancing Planets](https://link.springer.com/article/10.1007/s00283-025-10474-2),” *The Mathematical Intelligencer*, 2025. Places modular curve stitching in a broader mathematical and historical treatment.
- Amit Patel, “[Mathologer’s multiplication on a circle with modulo](https://www.redblobgames.com/x/1847-mathologer-modulo-circle/),” 2018. An interactive prior implementation with explicit links to other versions.
- Burkard Polster (Mathologer), “[Times Tables, Mandelbrot and the Heart of Mathematics](https://www.youtube.com/watch?v=qhbuKbxJsk8),” 2015. A major popular exposition and direct inspiration for many contemporary implementations.

The code and visual treatment here are original to this repository; the
underlying mathematical construction is established prior work.

## Published media

Not yet published.
