# 003 — Divisor field

This deterministic C++20 renderer lays consecutive integers on a row-major
grid. Each cell's opacity represents the number of positive divisors of its
integer. Bright cells therefore mark divisor-rich numbers; outlined record
cells are highly composite within the rendered prefix.

## Build and render

```bash
cmake -S . -B build
cmake --build build
./build/artifacts/003-divisor-field/divisor_field \
  --output out/003-divisor-field.svg \
  --count 250000 --columns 500 --scale sqrt --ceiling 168 --show-records
```

The renderer uses a divisor-count sieve: each possible divisor increments all
of its multiples. Cells with the same count are combined into a compact SVG
path, avoiding one document element per integer. No image library or random
seed is required.

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

`--columns` is perceptually powerful: it determines which integers share a
vertical alignment. Treat those alignments as a layout study, not evidence of a
new theorem. Use a fixed `--ceiling` when comparing different ranges.

The featured preset uses a 500×500 field, square-root contrast, and a fixed
ceiling of 168. Linear mapping makes ordinary values very dark; logarithmic
mapping lifts them enough to flatten the hierarchy. Square root is the selected
balance, while all three remain available for experimentation.

For a readable construction:

```bash
./build/artifacts/003-divisor-field/divisor_field \
  --output out/003-construction.svg --count 100 --columns 10 \
  --show-values --show-records
```

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
