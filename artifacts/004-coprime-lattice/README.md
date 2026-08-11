# 004 — Coprime Lattice

## What you are seeing

Each square marks an integer coordinate `(x,y)`. Gold points are visible from
the cyan origin: no other lattice point lies between them. Dim points are
blocked by a nearer point on the same ray.

## Principle

A point is visible from the origin exactly when `gcd(|x|,|y|)=1`. If the gcd is
`d>1`, then `(x/d,y/d)` is an integer point on the open segment to `(x,y)`.
Visible points are therefore primitive integer vectors.

In expanding centered squares, the visible fraction approaches `6/π²`, about
0.6079. A finite render reports its measured fraction; it does not claim exact
equality with the limit.

## Build and render

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/artifacts/004-coprime-lattice/coprime_lattice \
  --output out/004-coprime-lattice.svg --extent 120 \
  --encoding visibility --blocked-alpha 0.10 --point-size 0.72
```

No graphics dependency or random seed is required. The renderer groups equal
gcd values into compact SVG paths.

## Parameters and effects

| Parameter | Default | Visible effect |
| --- | --- | --- |
| `--extent` | `200` | Renders coordinates from `-N` through `N` on both axes. |
| `--encoding` | `visibility` | `visibility` separates primitive and blocked points; `gcd` varies blocked opacity by shared-factor size. |
| `--blocked-alpha` | `0.08` | Controls how strongly non-visible multiples remain in context. |
| `--point-size` | `0.72` | Sets mark size as a fraction of one lattice cell. |
| `--scale` | `sqrt` | Maps gcd contrast with `linear`, `sqrt`, or `log`. |
| `--ceiling` | auto | Fixes the gcd value mapped to maximum opacity in gcd mode. |
| `--width`, `--height` | `2160` | Set SVG canvas dimensions. |
| `--margin` | `80` | Adds space around the centered square lattice. |
| `--background` | `#080b14` | Sets the field color. |
| `--visible-color` | `#f6d35f` | Sets primitive-point color. |
| `--blocked-color` | `#27324a` | Sets non-visible-point color. |
| `--show-coordinates` | off | Labels coordinates when extent is at most 10. |
| `--output` | `out/004-coprime-lattice.svg` | Chooses output and creates parent directories. |

## Reproduction and experiments

[`presets/featured.json`](presets/featured.json) records the hero settings.
The renderer is deterministic.

- Start with `--extent 4 --show-coordinates` and inspect multiples along a ray.
- Set `--blocked-alpha 0` for primitive points only, then restore context.
- Compare extent 50, 100, 200, and 500; watch the measured fraction approach
  `6/π²`.
- Switch to `--encoding gcd` and compare contrast scales under a fixed ceiling.
- Change point size without changing extent to separate arithmetic structure
  from graphic density.

## References

- [NIST DLMF §27.2, Functions of Number Theory](https://dlmf.nist.gov/27.2)
- Michael Baake, Robert V. Moody, and Peter A. B. Pleasants, “Diffraction from
  visible lattice points and k-th power free integers,” *Discrete Mathematics*
  221 (2000), 3–42,
  [DOI 10.1016/S0012-365X(99)00384-2](https://doi.org/10.1016/S0012-365X(99)00384-2).
- Austin Goodrich, Aba Mbirika, and Jasmine Nielsen, “[New methods to find
  patches of invisible integer lattice points](https://arxiv.org/abs/1805.03186),” 2018.
