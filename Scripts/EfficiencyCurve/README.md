# EfficiencyCurve — relative γ‑ray efficiency from calibration spectra

This utility computes **relative detection efficiency curves** (plus a smooth fit) from a summed γ‑ray spectrum produced by calibration sources **¹⁵²Eu** or **¹³³Ba**. It reads a ROOT file containing a spectrum histogram, fits known γ lines (handling multiplets), and writes the resulting graphs and fit function to an output ROOT file.

> Directory: `Scripts/EfficiencyCurve/`  
> Entry point: `main.cxx` → class `SpectrumAnalyzer` (`SpectrumAnalyzer.cxx`)

---

## Features
- Loads a spectrum histogram (`TH1`) named **`energy_sum`** from a ROOT file (configurable via `--spec-name=...`).
- Fits known γ‑ray lines for **¹⁵²Eu** or **¹³³Ba**; nearby lines are grouped as multiplets (default proximity 6.5 keV).
- Produces graphs:
  - **Relative efficiency** vs energy → `relative_eff_graph` (`TGraphErrors`)
  - **Gaussian σ** vs energy → `sigma_graph` (`TGraphErrors`)
  - **Low‑energy tail τ** vs energy → `tau_graph` (`TGraphErrors`)
- Optionally shows fit canvases for interactive inspection (`--debug-canvas`).
- Persists / recalls fit parameters per input file to speed re‑runs.
- Fits a smooth efficiency parameterization in **50–1500 keV**:
  \[
  \varepsilon(E) \approx \frac{a_0 + a_1 \ln E + a_2 (\ln E)^2 + a_3 (\ln E)^3 + a_4 (\ln E)^5 + a_5 (\ln E)^7}{E}
  \]

---

## Requirements & build
- **ROOT** with graphics (Rint/GUI). On headless systems, omit `--debug-canvas`.
- Part of the **agataselector** project; needs the dynamic libraries of the main project.

Compile the main project enabling the scripts:
```bash
cmake -DBUILD_SCRIPTS=On .
```

---

## Usage
```
./efficiency_curve [OPTIONS] <spectrum.root> [more.root ...]
```

**Options**
- `--eu` / `--ba`  
  Select calibration set (**required: pick one**). Chooses the level scheme and intensity list:
  - Eu: `files/adoptedLevels152Sm_withalpha.csv`, `files/intensity152Eu.txt`
  - Ba: `files/adoptedLevels133Cs.csv`, `files/intensity133Ba.txt`
- `--spec-name=<name>`  
  Histogram to read (default: `energy_sum`).
- `--use-main-transitions`  
  Restrict to lines with intensity **> 0.5%**.
- `--read-from-file`  
  Reuse stored initial fit parameters for the given input file.
- `--debug-canvas`  
  Display diagnostic fit canvases.


**Examples**
```bash
# 152Eu run with canvases
./efficiency_curve --eu --debug-canvas data/eu_run1.root

# Batch 133Ba runs, only main transitions
./efficiency_curve --ba --use-main-transitions data/ba_*.root

# Re‑run reusing stored initial parameters
./efficiency_curve --eu --read-from-file data/eu_run1.root

# Non‑standard histogram name
./efficiency_curve --ba --hist-name=hpge_sum data/ba_cal.root
```

---

## Inputs & assumptions
- **ROOT file** with a histogram named `energy_sum` (`TH1D` preferred; `TH1F` is auto‑copied into `TH1D`).
- **Line lists / intensities** located in `files/` (see above). Intensity files are tab‑separated: **energy [keV]**, **intensity [%]**.
- Transitions with intensity `< 0.1%` are skipped by default; the `--use-main-transitions` flag raises the threshold to 0.5%.

> **Normalization model** (see `GenerateRelativeEffGraph()`):
> - `source_activity = 15524.`
> - `acq_time = 100.`
> - `t₁/₂(¹⁵²Eu) = 13.517 years`
> - `ndays = -222.` (effective age offset)
> The event count `nevts = acq_time × source_activity × exp(-ndays/τ)` with `τ = t₁/₂/ln 2` rescales the areas; the graph points and errors are divided by `nevts`.

These are harcoded at the moment and need to be changed in the code.

---

## Outputs
For each input file `<fname>.root` the program creates:
- **`eff_curve_<fname>.root`** containing:
  - `relative_eff_graph` — relative efficiency vs energy (`TGraphErrors`)
  - `sigma_graph` — Gaussian σ vs energy (`TGraphErrors`)
  - `tau_graph` — low‑energy tail τ vs energy (`TGraphErrors`)
  - `effcurve` — the smooth fit function (`TF1`) from `FitEffCurve()`
- **`data_<N>.root`** — intermediate save of `relative_eff_graph` during processing (index `N` is the internal call counter).
- **Parameter cache** (created unless `--read-from-file` is used and found):  
  `files/fitparams_relative_<fname-without-.root>.txt`

---

## Method (high level)
1. Load `TH1D` spectrum (copy from `TH1F` if needed) and compute `Sumw2()`.
2. Group nearby catalog lines into multiplets (ΔE < 6.5 keV).
3. Configure the fitter:
   - `Fitter(energies, Fitter::Tail::NOTAIL, 10)` → then `SetSpec(spect)`
   - First‑order background; limits set via `SetFitLimits(10,10)`, `SetSigmaLimits({0.8,3},1)`, `SetTauLimits({1e-4,0.3})`
   - Relative amplitudes fixed to catalog intensities; optional canvases enabled if requested
   - Initial τ set to 0.01
4. Fit each line/multiplet; filter out points with large relative error (> 7%) or blacklisted energies.
5. Build graphs for efficiency, σ, and τ; apply normalization by `nevts`.
6. Fit the smooth curve (`TF1 effcurve`) in **50–1500 keV** and store it.

---

## Troubleshooting
- **`File not found` / `Spectrum <name> not found`**  
  Check the input path and the histogram name (`--hist-name=...`).
- **No points in `relative_eff_graph`**  
  Loosen filters (remove `--use-main-transitions`) or verify intensity files.
- **GUI issues on servers**  
  Skip `--debug-canvas`; graphics libs are still initialized but canvases won’t appear without a display.

---

## Customization hints
- **Intensity threshold & proximity** → `GenerateRelativeEffGraph()` (`near_peak_threash = 6.5`).
- **Normalization constants** → `source_activity`, `acq_time`, `ndays`, `t12` block.
- **Fit behaviour** → `Fitter` configuration (`SetBkgOrder`, `SetSigmaLimits`, `SetTauLimits`, `SetRelativeAmplitudeFixed`, etc.).
