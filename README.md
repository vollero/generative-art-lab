# Generative Art Lab

Reproducible generative and algorithmic artworks built with C and C++.

Each artwork in this repository includes:

- source code and exact build/run instructions;
- an explanation of the algorithm and its artistic idea;
- a parameter guide describing visible effects;
- deterministic seeds or other reproduction inputs;
- export instructions for still images and social video;
- links to the videos in which the artwork appears.

## Start here

The collection is organized by theme rather than by social platform:

| Theme | Examples |
| --- | --- |
| Number theory | prime spirals, modular multiplication, divisor fields |
| Geometry | tilings, circle packing, Voronoi systems, curve families |
| Simulation | particles, reaction-diffusion, cellular automata, fluids |
| Signals | oscillators, Fourier drawing, harmonographs, interference |
| Chance and emergence | random walks, noise fields, stochastic growth |
| Image and video | feedback, pixel sorting, slit scan, motion studies |

## Artifacts

| ID | Artwork | Theme | Status |
| --- | --- | --- | --- |
| 001 | [Modular Times Table](artifacts/001-modular-times-table/) | Number theory | In production |

See the [artifact specification](docs/artifact-specification.md) for the common
format.

## Repository layout

```text
artifacts/<id>/       one self-contained artwork
common/               small reusable C/C++ utilities
docs/                 project-wide explanations and contributor guidance
scripts/              build, render, and validation helpers
third_party/          dependency notices or vendored, permitted dependencies
```

Every artifact owns its documentation, presets, source, and generated-media
manifest. Large rendered images and videos are releases or external media, not
ordinary Git objects.

## Build philosophy

- C++20 is the default language; C is welcome where it makes the idea clearer.
- CMake provides a portable build entry point.
- Seeds and presets make outputs reproducible.
- CPU implementations come first; GPU versions may be added when they teach
  something useful or materially improve the work.
- Dependencies are deliberately small and documented per artifact.

Artifact 001 is the first runnable example; its README contains the tested build,
render, parameter, and export instructions.

Artifact-specific export tools live beside their source because presets and
media requirements can differ between pieces.

## Licensing

Unless a file says otherwise, source code is licensed under the MIT License.
Written explanations and original rendered media are licensed under
CC BY 4.0. Third-party assets retain their own licenses; see their notices.

## Follow the project

Video and social links will be listed here as the first artifacts are released.
