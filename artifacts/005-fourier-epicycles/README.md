# 005 — Fourier Epicycles

## What you are seeing

A closed curve is sampled as complex values `z = x + iy`. The cyan vectors are
Fourier terms connected tip to tail. Each rotates at an integer frequency; the
last endpoint traces the gold reconstruction.

## Principle

For `N` samples, the renderer computes the normalized discrete Fourier
coefficients

`c_k = (1/N) Σ z_j exp(-2πikj/N)`.

At time `t`, it reconstructs `z(t) = Σ c_k exp(2πikt)`. A term's coefficient
magnitude is its circle radius, its argument is its initial angle, and `k` is
its rotation frequency. Indices above `N/2` are represented as negative
frequencies. Keeping every term recovers the input samples within floating-point
tolerance; keeping fewer produces an approximation.

## Build and render

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/artifacts/005-fourier-epicycles/fourier_epicycles \
  --output out/005-fourier-epicycles.svg --samples 256 --terms 32 \
  --time 0.72 --resampling arc-length --term-order amplitude
```

The renderer is dependency-free and deterministic. Its built-in heart curve is
generated parametrically. To use another closed outline, pass a text file with
one `x,y` point per line using `--input FILE`; repeating the first point is
optional.

## Parameters and effects

| Parameter | Default | Visible effect |
| --- | --- | --- |
| `--samples` | `256` | Sets source-signal resolution and the maximum term count. |
| `--terms` | `32` | Retains the strongest/requested coefficients; more terms usually reduce RMS error and add smaller circles. |
| `--time` | `0.72` | Chooses the chain position within one periodic traversal, in `[0,1)`. |
| `--trace-length` | `1` | Sets reconstructed history from no trail to a full cycle. |
| `--resampling` | `arc-length` | `arc-length` spaces samples spatially; `input` preserves vertex-density bias. |
| `--term-order` | `amplitude` | Changes circle-chain layout only; `frequency` and `amplitude` orders have the same endpoint for the same retained terms. |
| `--hide-circles` | off | Removes construction circles while retaining vectors. |
| `--hide-source` | off | Removes the dim reference outline. |
| `--width`, `--height` | `2160` | Set SVG canvas dimensions. |
| `--margin` | `180` | Controls artwork scale and outer clearance. |
| `--input` | built-in | Loads a custom CSV/polyline instead of the built-in curve. |
| `--output` | `out/005-fourier-epicycles.svg` | Chooses output and creates parent directories. |

The reported RMS error is measured at the sampled times in normalized curve
coordinates. It compares presets numerically but does not replace visual review
between samples.

## Reproduction experiments

- Compare `--terms 1`, `8`, `32`, and `256`; record the reported RMS error.
- Switch term ordering and observe that the chain changes but its endpoint does
  not when the same terms are retained.
- Provide an unevenly sampled polyline, then compare both resampling modes.
- Change `--time` in equal increments to generate animation frames; omit the
  duplicate frame at time `1`, which equals time `0`.

## References

- [NIST DLMF §1.8, Fourier Series](https://dlmf.nist.gov/1.8)
- [NIST DLMF §3.11, Approximation Techniques](https://dlmf.nist.gov/3.11)
- [FFTW 3.3.11, The 1d Discrete Fourier Transform](https://fftw.org/doc/The-1d-Discrete-Fourier-Transform-_0028DFT_0029.html)
