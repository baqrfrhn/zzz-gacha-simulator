# ZZZ Gacha Simulator

A desktop app that simulates pulling on Zenless Zone Zero banners and computes the **exact analytical probability distribution** next to a **Monte Carlo estimate**, so you can check what the math predicts against what the RNG actually does.

## What it does

- Simulates an **Agent** banner, a **W-Engine** banner, or an **Agent → W-Engine** combo (finish the agent target first, then the w-engine target).
- Computes the full probability mass function **analytically** (via dynamic programming, not an approximation), then overlays a Monte Carlo histogram on top.
- Reports **mean, median, and variance** for total pulls needed — both analytic and simulated — plus a much larger statistics panel (percentiles, skewness, kurtosis, entropy, …).
- Interactive GUI: banner type, mindscape rank, starting pity, guarantee status, simulation count, and pull budget all live-tweakable and re-runnable in one click.
- PMF ↔ CDF toggle with snap-to-point hover readouts, a pull-budget marker, and a per-run pull-history viewer.

## Gacha mechanics modeled

| Parameter | Agent Banner | W-Engine Banner |
| --- | --- | --- |
| Base S-rank rate | 0.6% | 1.0% |
| Hard pity | 90 | 80 |
| Soft pity start | 74 | 65 |
| Ramp rate (per pull after soft pity) | 6% | 6% |
| Win rate | 50% (50/50) | 75% (75/25) |

These defaults live in `include/types.h` as `kAgentConfig` and `kWEngineConfig` and match the in-game rates; every one of them can be changed in the UI.

### Soft pity

The soft-pity system is a piecewise per-pull rate — flat base rate up to the soft-pity threshold, then a linear ramp, capped at 100% at hard pity. The exact formula the code uses (`simulation.cpp`, `pmf_calculator.cpp`), with `p` the 1-indexed pity counter:

```
rate(p) = baseRate                                      for p < softPityStart
rate(p) = baseRate + rampRate * (p - (softPityStart - 1))   for softPityStart <= p < hardPity
rate(p) = 1.0                                           for p == hardPity
```

With the Agent defaults that means at pull 74 the rate jumps to `0.6% + 6% = 6.6%`, and at pull 90 it's a guaranteed hit.

### The 50/50 (75/25) guarantee

Every S-rank either **wins** (advances your copy count) or **loses** (sets a guarantee flag so the *next* S-rank is a guaranteed win). Losing never consumes a copy — it just inserts one extra guaranteed S-rank into the chain. Multiple copies are modeled by chaining these events: the simulation keeps pulling until the win count reaches the target copies.

## How the math works

### Analytical PMF — dynamic programming

The analytic distribution is computed exactly with a Markov-chain-style dynamic program over the state `(copies acquired, guarantee flag, pity counter)`. The state grid is `OutcomeGrid3D = [copies][guarantee][pity]` (see `include/types.h`), seeded with probability 1 at the starting state, then advanced one pull at a time:

- **miss** → advance the pity counter;
- **hit with no guarantee** → win with probability `winRate` (gain a copy, reset pity) or lose with `1 - winRate` (set guarantee, reset pity);
- **hit with guarantee** → guaranteed win.

Mass is written into `pmf[pullsDone]` the moment a copy count reaches `numCopiesNeeded`. Because the DP walks every path, the result is the true PMF for the pity/ramp/guarantee model — there's no closed-form shortcut being used.

### Monte Carlo simulation

Each trial pulls until it reaches the target copies and records total pulls. The engine uses `std::mt19937` (Mersenne Twister), with each worker thread seeding its own generator from `std::random_device` through a `seed_seq`. The simulated PMF is just the normalized histogram of trial outcomes.

Simulations are parallelized across `std::thread::hardware_concurrency()` threads (falls back to 8). Each thread writes to a disjoint slice of the result vector, so there's no locking or contention.

### Statistics

`stats_calculator.cpp` reduces both distributions to mean, median, and variance. The statistics panel then derives a richer set on the fly from the PMF: mode, standard deviation, coefficient of variation, mean absolute deviation, min/max/range, P5–P99 percentiles, IQR, skewness, excess kurtosis, and entropy.

### Combo mode (Agent → W-Engine)

The two legs are independent, so the joint analytic PMF is the **convolution** of the two leg PMFs (`i + j` pull totals). For the simulated side, run *i* of the Agent leg is paired with run *i* of the W-Engine leg and their totals summed. The result is a single "total pulls for both targets" distribution.

## Architecture

```
zzz-gacha-simulator/
├── include/
│   ├── types.h              # core types, banner configs, PMF/grid aliases
│   ├── app_state.h          # AppState + worst-case pull helpers
│   ├── simulation.h         # Simulation class (MC + combo mode)
│   ├── pmf_calculator.h     # analytic + simulated PMF API
│   ├── stats_calculator.h   # mean / median / variance
│   ├── theme.h              # ThemeConfig, three themes, Theme API
│   ├── gui.h                # Gui class (window setup, main loop)
│   └── ui_panels.h          # panel + modal class declarations
├── src/
│   ├── main.cpp             # entry point — constructs Gui, starts the loop
│   ├── gui/
│   │   ├── gui.cpp          # window setup, dashboard layout, title bar
│   │   ├── theme.cpp        # ImGui/ImPlot colour + style application
│   │   └── panels/
│   │       ├── parameter_panel.cpp   # banner type, rank, pity, guarantee, run
│   │       ├── budget_panel.cpp      # CDF at a given pull budget
│   │       ├── statistics_panel.cpp  # the side-by-side stats table
│   │       ├── plot_panel.cpp        # PMF/CDF plot, hover, budget marker
│   │       └── history_modal.cpp     # per-run pull history viewer
│   └── math/
│       ├── simulation.cpp    # Monte Carlo engine + combo convolution
│       ├── pmf_calculator.cpp# analytic DP + histogram
│       └── stats_calculator.cpp
├── resources/
│   ├── Zenless_Zone_Zero_logo.ico / .png   # window icon
│   ├── rabbit.png
│   ├── resource.rc          # embeds the .ico on Windows
│   └── License.txt
├── LICENSE
└── README.md
```

### Math layer (`src/math/`)

- **`simulation.cpp`** — owns the `Simulation` class. `RunFullSimulation()` resizes the history buffer, fans the trials out across threads, then calls `ComputePmfStats()`. `Simulation::Run()` is one trial's inner loop (the per-pull rate check, the 50/50 roll, the pity reset). `RunCombined()` runs both legs and convolves them.
- **`pmf_calculator.cpp`** — `CalcAnalyticalPmf()` runs the DP; `CalcSimulatedPmf()` builds the normalized histogram from `pullHistory`.
- **`stats_calculator.cpp`** — `CalcStatistics()` and the mean/median/variance helpers that back `StatisticsData`.

### GUI layer (`src/gui/`)

- **`gui.cpp`** — sets up the raylib window (1600×900, min 1280×720, 165 FPS target, HiDPI + resizable), loads the window icon from `RESOURCES_PATH`, initializes rlImGui and ImPlot, and runs the main loop. `DrawDashboard()` lays out a 36/64 two-column split: parameters, budget, and statistics on the left; plot and pull history on the right.
- **`theme.h` / `theme.cpp`** — a `ThemeConfig` struct holds every colour that differs between themes. Three themes are defined: **Agent** (navy + gold), **W-Engine** (purple + lime), and **Combo** (graphite + champagne gold). `Apply()` does one-time style setup; `ApplyColors()` swaps the whole ImGui palette every frame. Plot-line colours (analytic vs simulated) are per-theme too.

### Panels (`src/gui/panels/`)

- **`parameter_panel.cpp`** — the three banner-type buttons (which also switch the theme), a simulations slider (100k–10M), mindscape-rank radio buttons (M0–M6), a "pulls until hard pity" slider, and the guarantee checkbox ("Lost previous 50/50" / "Lost previous 75/25"). Combo mode splits this into independent AGENT and W-ENGINE sections. Returns `true` when "Run Simulation" is clicked.
- **`budget_panel.cpp`** — a pull-budget slider plus the analytic CDF % at that budget (coloured green/amber/red), the simulated CDF %, and the worst-case guarantee pull count.
- **`statistics_panel.cpp`** — the three-column "Stat / Analytic / Simulated" table with the full metric set listed above.
- **`plot_panel.cpp`** — renders the PMF (or CDF) with ImPlot, plots analytic vs simulated as staircase lines, draws the red pull-budget marker, and implements hover snapping with a crosshair and a "Pulls / Analytic % / Simulated %" tooltip.
- **`history_modal.cpp`** — a popup table of every Monte Carlo run: run number, total pulls + event count, and the win/loss chain (e.g. `Won at 78 → Lost at 74 → Won at 80`). Combo mode shows Agent and W-Engine tabs, with a "jump to sim #" control, scroll syncing between tabs, and `ImGuiListClipper` for large trial counts.

## Building

Dependencies:

- C++17 compiler (MSVC, Clang, or GCC)
- [raylib](https://www.raylib.com/) 5.0+ (graphics/input)
- [rlImGui](https://github.com/raylib-extras/rlImGui) (ImGui ↔ raylib glue)
- [Dear ImGui](https://github.com/ocornut/imgui) (docking branch recommended)
- [ImPlot](https://github.com/epezent/implot) (plotting)

```bash
git clone https://github.com/baqrfrhn/zzz-gacha-simulator.git
cd zzz-gacha-simulator

# Build with CMake (add the deps via vcpkg or vendor them as submodules)
cmake -B build -S .
cmake --build build --config Release
```

Two build-system details the code depends on:

- **`RESOURCES_PATH`** — a compile-time macro pointing at the `resources/` directory. `gui.cpp` uses it to locate the window icon at runtime (`RESOURCES_PATH + "Zenless_Zone_Zero_logo.png"`).
- **`resources/resource.rc`** — embeds the ZZZ logo `.ico` into the executable on Windows. On Linux/macOS you'd skip the `.rc` (it's Windows-only) and load the `.png` icon at runtime instead.

## Usage

Run the executable. The dashboard has four panels:

1. **Parameters** — banner type, mindscape rank, starting pity, guarantee toggle, simulation count.
2. **Budget** — enter your pull budget to see the probability of hitting your target within it.
3. **Statistics** — mean/median/variance and the extended metrics, analytic vs simulated side by side.
4. **Plot** — the PMF (or CDF) with both curves overlaid; hover any pull count for exact values.

The **View Pull History** button opens a modal with the raw win/loss outcome of every individual run.

## Limits & assumptions

- Maximum **7 copies** (M0–M6) and **14 S-rank events** per run (`maxCopiesPossible`, `maxNumberOfSRanks` in `types.h`).
- Simulation count is bounded to **100,000–10,000,000** in the UI.
- The single-banner PMF/histogram is sized to `14 × hardPity` entries (1261 for Agent, 1121 for W-Engine); the DP's pity dimension is a fixed 90-slot array reused for both banner types.
- "Pulls until hard pity" is the UI-facing inverse of `startingNumPulls`: internally `startingNumPulls = hardPity - pullsUntilHardPity`.

## License

MIT — see [LICENSE](LICENSE).
