# Generative Art Lab

Reproducible generative and algorithmic artworks built with C and C++.

Each artwork in this repository includes:

- source code and exact build/run instructions;
- an explanation of the algorithm and its artistic idea;
- a parameter guide describing visible effects;
- deterministic seeds or other reproduction inputs;
- instructions for rendering still images or algorithmic animation.

## Start here

The collection is organized by mathematical and computational theme:

| Theme | Examples |
| --- | --- |
| Number theory | prime spirals, modular multiplication, divisor fields |
| Geometry | tilings, circle packing, Voronoi systems, curve families |
| Simulation | particles, reaction-diffusion, cellular automata, fluids |
| Signals | oscillators, Fourier drawing, harmonographs, interference |
| Chance and emergence | random walks, noise fields, stochastic growth |
| Image processing | feedback, pixel sorting, slit scan, motion studies |

## Artifacts

| ID | Artwork | Theme | Status |
| --- | --- | --- | --- |
| 001 | [Modular Times Table](artifacts/001-modular-times-table/) | Number theory | Available |
| 002 | [Ulam Prime Spiral](artifacts/002-ulam-prime-spiral/) | Number theory | In development |

## Repository layout

```text
artifacts/<id>/       one self-contained artwork
third_party/          dependency notices or vendored, permitted dependencies
```

Every artifact owns its explanation, parameter guide, presets, source code,
tests, and generic render helpers.

## Build philosophy

- C++20 is the default language; C is welcome where it makes the idea clearer.
- CMake provides a portable build entry point.
- Seeds and presets make outputs reproducible.
- CPU implementations come first; GPU versions may be added when they teach
  something useful or materially improve the work.
- Dependencies are deliberately small and documented per artifact.

Artifact 001 is the first runnable example; its README contains the tested build,
render, parameter, and export instructions.

Artifact-specific render tools live beside their source because algorithms and
parameter presets differ between pieces.

## Support the project

If these artifacts, explanations, or reusable implementations are useful to
you, you can support continued work at
[Buy Me a Coffee](https://buymeacoffee.com/vollero).

## Licensing

Unless a file says otherwise, source code is licensed under the MIT License.
Written explanations and original rendered outputs are licensed under CC BY
4.0. Third-party assets retain their own licenses; see their notices.
