# 002 — Ulam Prime Spiral

## What you are seeing

Prime numbers appear as points on a square spiral of consecutive integers.
Although primality is a one-dimensional property, the arrangement exposes long
diagonal streaks and gaps instead of uniform visual noise.

## Idea

Place `1` at the center of a square grid. Put `2` to its right, turn upward for
`3`, then continue counterclockwise in an expanding square spiral. Mark a cell
only when its integer is prime.

The spiral does not make primes more regular. It makes some arithmetic
relationships spatially adjacent. Values along a straight diagonal are given
by quadratic expressions, and some quadratic polynomials produce primes more
often over particular ranges than neighboring polynomials. Those differences
become visible as prime-rich and prime-poor tracks.

## Algorithm

1. Compute primality for every value in the requested range with the Sieve of
   Eratosthenes.
2. Start at grid coordinate `(0, 0)` with the chosen center value.
3. Move right, up, left, and down; increase the segment length after every two
   turns to form the square spiral.
4. For each prime, map the grid coordinate to the SVG canvas and draw a circle.
5. Optionally draw composite values at low opacity to reveal the underlying
   square lattice and traversal density.

For maximum value `N`, the sieve uses `O(N)` bits and `O(N log log N)` time.
Traversal and output are linear in the number of grid cells.

## Build and run

Requirements: CMake 3.20+ and a C++20 compiler. No graphics dependency is
required for SVG output.

From the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/artifacts/002-ulam-prime-spiral/ulam_prime_spiral \
  --output out/002-featured.svg \
  --side 501 --start 1 --width 2160 --height 2160 \
  --margin 0.045 --radius 0.42
```

Run the full test suite with `ctest --test-dir build`.

## Parameters

| Parameter | Type / range | Default | Visible effect | Interactions |
| --- | --- | --- | --- | --- |
| `--side` | odd integer, 3–3001 | 1001 | Expands the number range and reveals longer diagonal structures. | Larger grids shrink each point at fixed canvas size and increase SVG/sieve size. |
| `--start` | integer, 0–10000000 | 1 | Changes the integer at the center and therefore which quadratic tracks cross the view. | The sieved maximum is `start + side² - 1`; large starts cost more memory. |
| `--width`, `--height` | integer, 64–16384 | 2160 | Set canvas dimensions and whitespace outside the square grid. | Cell size follows the shorter canvas dimension. |
| `--margin` | real, 0–0.25 | 0.045 | Adds space between the outer spiral and canvas edge. | Higher margins reduce cell and point size. |
| `--radius` | real, (0, 0.5] | 0.34 | Changes each point from sparse dust to a nearly filled cell. | At high values, nearby prime tracks look heavier and may merge after rasterization. |
| `--background` | SVG/CSS color | `#070912` | Sets the field color and overall contrast. | Balance against both point colors. |
| `--prime-color` | SVG/CSS color | `#f4d35e` | Sets the marked-prime color. | High contrast makes sparse diagonal tracks easier to perceive. |
| `--composite-color` | SVG/CSS color | `#27324a` | Sets contextual non-prime points. | Visible only when composite alpha is above zero. |
| `--composite-alpha` | real, 0–1 | 0 | Reveals the full traversal/lattice. | Low values teach the construction; high values reduce prime contrast. |
| `--output` | path | `ulam-prime-spiral.svg` | Chooses the SVG destination. | Parent directories are created automatically. |

## Preset and reproduction

[`presets/featured.json`](presets/featured.json) records the featured values in
a tool-neutral format. The executable uses explicit command-line parameters so
the render command remains visible and reproducible without a JSON dependency.

There is no randomness and therefore no seed. Integer coordinates and a fixed
three-decimal SVG format make the vector output deterministic for identical
inputs.

## Experiments

- Start with side `21` and composite alpha `0.12` to see the number placement.
- Increase the side through `101`, `501`, and `1001` while keeping the canvas
  fixed; observe when individual primes become diagonal textures.
- Compare center values `1` and `41` at the same side length.
- Reduce radius to make density differences less likely to merge visually.
- Use a rectangular canvas to study framing without distorting the square grid.

## References

- The Smithsonian National Museum of American History, “[Numbers in a Spiral](https://www.si.edu/object/painting-numbers-spiral%3Anmah_694675).” Historical account of Ulam’s early-1960s sketch, MANIAC computation, 1964 publication, and Scientific American cover.
- MacTutor History of Mathematics, “[Ulam’s Spiral](https://mathshistory.st-andrews.ac.uk/Extras/Ulam_spiral/).” Explanation of the square spiral and the quadratic expressions associated with straight tracks.
- Arkadiusz Orłowski and Leszek J. Chmielewski, “[Ulam Spiral and Prime-Rich Polynomials](https://doi.org/10.1007/978-3-030-00692-1_45),” 2018. Image-analysis study of line segments and their prime-generating polynomials.
