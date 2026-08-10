# ZZZ Gacha Simulator

A desktop app that simulates pulling on Zenless Zone Zero banners — with both Monte Carlo simulation **and** exact analytical probabilities. Lets you compare the two side-by-side so you can see how the RNG actually behaves versus what the math predicts.

## What it does

- Simulates **Agent banners**, **W-Engine banners**, or a **combo** (Agent -> W-Engine) in one shot.
- Computes the full **probability mass function** analytically using the pity/ramp system, then overlays the Monte Carlo results on top.
- Outputs mean, median, and variance for total pulls needed — both analytic and simulated.
- Interactive GUI so you can tweak parameters (number of copies, starting pity, guarantee status, etc.) and re-run instantly.

## The gacha mechanics modeled

| Parameter | Agent Banner | W-Engine Banner |
|-----------|-------------|-----------------|
| Base S-rank rate | 0.6% | 1.0% |
| Hard pity | 90 | 80 |
| Soft pity start | 74 | 65 |
| Ramp rate (per pull after soft pity) | 6% | 6% |
| 50/50 win rate | 50% | 75% |

These defaults match the in-game rates. You can change them in the UI if you want to experiment with hypothetical banners.

## How it works

### Analytical PMF

The soft pity system is modeled as a piecewise probability: a flat base rate up to the soft pity threshold, then a linear ramp (`base + k × ramp`) for each subsequent pull until hard pity guarantees the drop. The analytical PMF is computed by convolving the single-S-rank distribution the required number of times (once per copy needed), accounting for the 50/50 — every loss injects an extra guaranteed S-rank with its own distribution.

### Monte Carlo simulation

Runs a configurable number of trials (default 1,000,000) using an Mersenne Twister RNG. Each trial pulls until it hits the target number of copies, recording how many total pulls it took. The simulated PMF is built from the histogram of trial outcomes.

### Statistics

Both the analytic and simulated distributions are reduced to mean, median, and variance so you can sanity-check the simulation against theory.

## Building

### Dependencies

- **C++17** compiler (MSVC, Clang, or GCC)
- **[raylib](https://www.raylib.com/)** 5.0+ (graphics/input)
- **[rlImGui](https://github.com/raylib-extras/rlImGui)** (ImGui integration for raylib)
- **[Dear ImGui](https://github.com/ocornut/imgui)** (docking branch recommended)

### Windows (MSVC)

```sh
# Clone with submodules if you're vendoring raylib/imgui, or install via vcpkg:
git clone https://github.com/baqrfrhn/zzz-gacha-simulator.git
cd zzz-gacha-simulator

# Build with CMake
cmake -B build -S .
cmake --build build --config Release
```

The resource file (`resources/resource.rc`) embeds the ZZZ logo icon into the executable.

### Linux / macOS

Not tested, but raylib + ImGui are cross-platform. You'll need to sort out the resource file (`.rc` is Windows-only — either skip it or use a `.png` icon at runtime).

## Usage

Run the executable. The main dashboard has four panels:

1. **Parameters** — banner type, number of copies, starting pulls, guarantee toggle, simulation count.
2. **Budget** — enter your current pulls + polychrome to see how likely you are to hit your target.
3. **Statistics** — mean, median, variance side-by-side (analytic vs simulated).
4. **Plot** — PMF chart with both curves overlaid; scroll through pull counts.

There's also a **pull history** modal where you can inspect the raw outcomes of individual simulation runs.

## Project structure

```
src/
  main.cpp          — entry point, starts the GUI
  gui/              — GUI implementation (panels, theme, rendering)
  math/             — simulation engine, PMF calculator, statistics
include/            — headers for all of the above
resources/          — icon, fonts, license text
```

## License

MIT — see [LICENSE](./LICENSE).
