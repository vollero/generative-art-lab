# 003 — Divisor Field

## What you are seeing

This deterministic C++20 renderer lays consecutive integers on a row-major
grid. Each cell's opacity represents the number of positive divisors of its
integer. Bright cells therefore mark divisor-rich numbers; outlined record
cells are highly composite within the rendered prefix.

## Idea

Every positive integer has a divisor count `d(n)`. Prime numbers have two
positive divisors, squares have an odd divisor count, and numbers with many
factor combinations have larger counts. The sequence is irregular when read
one number at a time.

Placing that sequence across rows makes the irregularity spatial. Brightness
encodes `d(n)`, while the column count decides where each row wraps. Changing
the width reorganizes the same arithmetic data, so apparent bands and
diagonals must be interpreted as layout-dependent views rather than new
number-theory claims.

## Algorithm

1. Allocate a divisor count for every integer through the rendered endpoint.
2. For each possible divisor `d`, increment the count of `d`, `2d`, `3d`, and
   every later multiple.
3. Select the requested range and find its maximum divisor count.
4. Map each count through a linear, square-root, or logarithmic contrast scale.
5. Place consecutive values in row-major order and encode the result as cell
   opacity.
6. Combine cells with the same count into compact SVG paths.
7. Optionally label small fields, outline record counts, or highlight one
   integer.

For endpoint `N`, the divisor-count sieve takes `O(N log N)` time in the
harmonic-sum sense and `O(N)` memory. SVG generation is linear in the number of
rendered cells.

## Build and run

Requirements: CMake 3.20+ and a C++20 compiler. No graphics dependency is
required for SVG output.

From the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/artifacts/003-divisor-field/divisor_field \
  --output out/003-divisor-field.svg \
  --count 250000 --columns 500 --scale sqrt --ceiling 168 --show-records
```

The renderer uses a divisor-count sieve: each possible divisor increments all
of its multiples. Cells with the same count are combined into a compact SVG
path, avoiding one document element per integer. No image library or random
seed is required.

Run all repository tests with:

```bash
ctest --test-dir build --output-on-failure
```

## Parameters and effects

| Parameter | Default | Effect |
| --- | --- | --- |
| `--start` | `1` | Changes the first integer without renumbering later cells. |
| `--count` | `65536` | Increases extent and the number of arithmetic samples. |
| `--columns` | `256` | Changes row wrapping and apparent bands without changing divisor counts. |
| `--width`, `--height` | `2160` | Set SVG canvas dimensions. |
| `--margin` | `80` | Adds empty space around the field. |
| `--gap` | `0.08` | Separates cells; values from `0` through less than `0.9`. |
| `--scale` | `sqrt` | `linear`, `sqrt`, or `log` contrast mapping. |
| `--ceiling` | auto | Fixes the divisor-count value mapped to maximum opacity. |
| `--show-records` | off | Outlines each new divisor-count record in white. |
| `--show-values` | off | Labels `n` and `d(n)` when at most 400 cells are rendered. |
| `--highlight` | off | Outlines one in-range integer in cyan for explanation frames. |
| `--background`, `--foreground` | dark/gold | Changes palette, not arithmetic selection. |
| `--output` | `out/003-divisor-field.svg` | Chooses the SVG destination; parent directories are created. |

`--columns` is perceptually powerful: it determines which integers share a
vertical alignment. Treat those alignments as a layout study, not evidence of a
new theorem. Use a fixed `--ceiling` when comparing different ranges.

The featured preset uses a 500×500 field, square-root contrast, and a fixed
ceiling of 168. Linear mapping makes ordinary values very dark; logarithmic
mapping lifts them enough to flatten the hierarchy. Square root is the selected
balance, while all three remain available for experimentation.

## Preset and reproduction

[`presets/featured.json`](presets/featured.json) records the featured range,
layout, canvas, contrast mapping, ceiling, palette, and record-outline setting
in a tool-neutral format. The executable deliberately uses explicit command
line options so the render command remains self-documenting without a JSON
runtime dependency.

The renderer contains no randomness. Integer divisor counts and fixed
three-decimal SVG coordinates make output deterministic for identical inputs.

For a readable construction:

```bash
./build/artifacts/003-divisor-field/divisor_field \
  --output out/003-construction.svg --count 100 --columns 10 \
  --show-values --show-records
```

## Experiments

- Compare `--columns 64`, `100`, `120`, `210`, `256`, and `500` while keeping
  `--start`, `--count`, and `--ceiling` fixed.
- Compare `--scale linear`, `sqrt`, and `log`. Linear emphasizes only the most
  divisor-rich values; logarithmic mapping lifts ordinary cells; square root is
  the featured balance.
- Render the first 100 integers with labels and verify `d(12)=6`, `d(24)=8`,
  `d(48)=10`, and `d(60)=12`.
- Shift `--start` while keeping count and columns fixed to crop a different
  part of the divisor sequence.
- Use `--highlight N` to explain a selected cell and `--show-records` to trace
  successive highly composite values.
- Change gap and palette while holding arithmetic parameters fixed to separate
  graphic treatment from divisor structure.

## Mathematical principle

The divisor function `d(n)` counts positive divisors. From a prime
factorization `n = p1^a1 ... pk^ak`, its value is
`(a1 + 1) ... (ak + 1)`. The renderer computes the same values with a sieve.

References:

- [NIST DLMF §27.2, Functions of Number Theory](https://dlmf.nist.gov/27.2)
- [OEIS A000005, number of divisors of n](https://oeis.org/A000005)
- S. Ramanujan, “Highly Composite Numbers,” *Proceedings of the London
  Mathematical Society* 14 (1915), 347–409,
  [DOI 10.1112/plms/s2_14.1.347](https://doi.org/10.1112/plms/s2_14.1.347)

Code is MIT licensed. Documentation and original renders follow the repository
content license.
