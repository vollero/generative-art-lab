# 006 — Modular Flow

## What you are seeing

Each point is a residue `x` modulo `n`. Its arrow points to `x² mod n` (or to
`x^e mod n` when the exponent changes). Because every point has exactly one
outgoing edge, repeated iteration eventually reaches a directed cycle. The
remaining points form transient trees flowing into those cycles.

This differs from Artifact 001: Modular Times Table draws one layer of
multiplication chords; Modular Flow follows the eventual behavior of a
nonlinear iterated map.

## Build and render

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/artifacts/006-modular-flow/modular_flow \
  --output out/006-modular-flow.svg --modulus 63 --exponent 2 \
  --encoding depth --depth-scale sqrt --edge-alpha 0.20
```

The dependency-free C++20 renderer classifies the functional graph, checks its
invariants, writes deterministic SVG, and reports cycle count, longest cycle,
and maximum transient depth.

## Parameters and effects

| Parameter | Default | Visible effect |
| --- | --- | --- |
| `--modulus` | `63` | Changes the residue set and therefore the entire graph. |
| `--exponent` | `2` | Iterates `x^e mod n`; this changes the mathematics, not only styling. |
| `--encoding` | `depth` | `role` separates cycle/transient nodes, `depth` varies transient opacity, and `basin` colors components. |
| `--depth-scale` | `sqrt` | Maps depth contrast with `linear`, `sqrt`, or `log` without changing classification. |
| `--edge-alpha` | `0.20` | Controls contextual strength of transient arrows. |
| `--vertex-size` | `7` | Sets residue mark size in SVG units. |
| `--highlight-start` | off | Highlights one residue's complete eventual orbit. |
| `--show-labels` | off | Labels residues for moduli up to 127. |
| `--width`, `--height` | `2160` | Set SVG canvas dimensions. |
| `--margin` | `150` | Sets outer clearance and circle radius. |
| `--output` | `out/006-modular-flow.svg` | Chooses output and creates parent directories. |

## Reproduction experiments

- Start with `--modulus 15 --show-labels --highlight-start 2` and follow one orbit.
- Compare moduli 15, 31, 63, and 127; use the reported summaries rather than
  assuming a universal prime/composite pattern.
- Switch among role, depth, and basin encodings while holding the graph fixed.
- Try exponents 3 and 5, noting explicitly that these are different maps.

## References

- [NIST DLMF §27.2, Functions of Number Theory](https://dlmf.nist.gov/27.2)
- [OEIS A023153, cycles of `x → x² mod n`](https://oeis.org/A023153)
- E. Blanton, S. Hurd, and J. McCranie, “On the Digraph Defined by Squaring
  Modulo n,” *Fibonacci Quarterly* 30(4), 322–334, 1992.
- J. J. Brennan and B. Geist, “Analysis of Iterated Modular Exponentiation,”
  *Designs, Codes and Cryptography* 13, 229–245, 1998,
  [DOI 10.1023/A:1008289605486](https://doi.org/10.1023/A:1008289605486).
