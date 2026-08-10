# Generative Art Lab

Reproducible generative and algorithmic artworks built with C and C++.

This public repository contains only the material needed to understand and
reproduce each artifact:

- source code and portable build instructions;
- the mathematical or computational principle;
- deterministic featured presets;
- rendering instructions;
- parameter descriptions and their visible effects;
- tests, references, and licensing notes.

Social editing, narration, captions, scheduling, analytics, and other
production-management material are intentionally not stored here.

## Published artifacts

| ID | Artwork | Principle | Release |
| --- | --- | --- | --- |
| 001 | [Modular Times Table](artifacts/001-modular-times-table/) | Modular multiplication turns straight chords into curved envelopes. | [`artifact-001-v1.0.0`](https://github.com/vollero/generative-art-lab/tree/artifact-001-v1.0.0/artifacts/001-modular-times-table) |
| 002 | [Ulam Prime Spiral](artifacts/002-ulam-prime-spiral/) | Primes placed on a square integer spiral expose diagonal tracks. | [`artifact-002-v1.0.0`](https://github.com/vollero/generative-art-lab/tree/artifact-002-v1.0.0/artifacts/002-ulam-prime-spiral) |
| 003 | [Divisor Field](artifacts/003-divisor-field/) | Divisor counts become brightness in a woven integer-grid texture. | [`artifact-003-v1.0.0`](https://github.com/vollero/generative-art-lab/tree/artifact-003-v1.0.0/artifacts/003-divisor-field) |

The links in the artwork column follow the current branch. Release links are
immutable reproduction targets.

## Build everything

Requirements: CMake 3.20+ and a C++20 compiler.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

No graphics library is required for the core SVG renderers. Each artifact
README provides its exact command, featured preset, parameter guide, and any
optional raster/video export dependencies.

## Repository layout

```text
artifacts/<id>/
├── src/              C or C++ renderer
├── presets/          deterministic reproduction inputs
├── CMakeLists.txt    target and artifact tests
├── README.md         principle, rendering, and parameter effects
└── LICENSES.md       artifact-specific licensing notes
```

Some artifacts also contain generic render helpers under `scripts/`. Generated
files belong in ignored `out/`, `renders/`, or `build/` directories.

## Design rules

- C++20 is the default; C is welcome when it makes the idea clearer.
- CPU implementations come first.
- Renderers produce deterministic, inspectable output.
- Presets record every input needed for a featured result.
- Parameter documentation distinguishes arithmetic changes from purely visual
  or layout changes.
- Dependencies remain small and are documented where used.

## Support the project

If these artifacts, explanations, or reusable implementations are useful to
you, you can support continued work at
[Buy Me a Coffee](https://buymeacoffee.com/vollero).

## Licensing

Unless a file says otherwise, source code is licensed under the MIT License.
Written explanations and original rendered outputs are licensed under CC BY
4.0. Third-party assets retain their own licenses; see their notices.
