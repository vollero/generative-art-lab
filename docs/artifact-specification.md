# Artifact specification

An artifact is a versioned, independently reproducible artwork. Its directory
name is a stable three-digit sequence and a short slug, for example
`001-modular-times-table`.

## Required files

```text
artifacts/001-example/
├── README.md
├── CMakeLists.txt
├── src/
├── presets/
│   └── featured.json
├── media/
│   └── manifest.md
└── LICENSES.md
```

The artifact README must contain:

1. **What you are seeing** — a short visual description.
2. **Idea** — the mathematical, physical, or artistic principle.
3. **Algorithm** — enough detail to implement it independently.
4. **Build and run** — tested commands and dependencies.
5. **Parameters** — type/range, default, visual effect, and interactions.
6. **Presets and seeds** — exact inputs used for published renders.
7. **Export** — resolution, frame rate, duration, codec, and color assumptions.
8. **Experiments** — useful directions for changing the piece.
9. **Credits and references** — inspirations, papers, assets, and licenses.
10. **Published media** — canonical links for each platform.

## Reproducibility contract

- The same release, preset, seed, compiler family, and dependency versions
  should produce the same composition. Pixel-identical output is preferred but
  not promised across platforms unless explicitly tested.
- Randomness must come from an explicitly selected and recorded seed.
- A featured render records its commit SHA and command line in `media/manifest.md`.
- Defaults create a useful result without undocumented input files.
- Parameter validation fails with a helpful message rather than silently
  accepting invalid values.

## Parameter table

Use this format in every artifact README:

| Parameter | Type / range | Default | Visible effect | Interactions |
| --- | --- | --- | --- | --- |
| `example` | float, 0–1 | 0.5 | Describe what increases and decreases do. | Note coupled parameters. |

## Media policy

Keep thumbnails and a few compressed reference images in Git. Publish large
lossless renders and videos as GitHub Release assets or through the relevant
media platform. Never commit licensed music unless redistribution is allowed.

