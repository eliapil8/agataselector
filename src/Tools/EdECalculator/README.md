# EdECalculator

This folder contains the portable calculation and detector adapters used to
generate reaction-constrained E-dE loci.

`EdECalculator` is detector independent. Each instance owns exactly one binary
reaction and accepts an ordered target/backing stack plus detector layer
descriptions. It calculates the reaction at the front, nominal position, and
back of the selected reaction layer and transports the detected fragment
through the material stack and detector.

The portable core has no dependency on selector configuration, OSCAR LUT
types, histograms, or ROOT graph classes. `EdECalculatorOscar` is the separate
OSCAR adapter. It translates the selector target/backing stack and every LUT
telescope into core inputs, owns one calculator per reaction, and writes open
`TGraphAsymmErrors` overlays.

Detector integrations belong in this same folder but remain separate from the
core calculator. `EdECalculatorOscar` will translate selector and OSCAR LUT
configuration into the generic inputs and will own ROOT visualization.

## Reaction definitions

The constructor requires one reaction definition. A binary reaction can be
created by specifying the detected fragment. A transfer reaction can instead
specify `(transferredA, transferredZ)` moving from the projectile to the
target-like product. Multiple reactions are represented by multiple calculator
instances; `EdECalculatorOscar` owns and coordinates that collection.

Excitation energies may be supplied as an arbitrary vector (for known nuclear
levels) or produced with `MakeExcitationGrid` (for a regular scan). Each result
contains the front, nominal-depth, and back calculations plus asymmetric depth
errors for the reaction energy and each detector-energy coordinate. This keeps
the core independent of the eventual choice of axes in `TGraphAsymmErrors`.

## Failure policy

Call `Validate` once after constructing all detector descriptions. Missing
nuclear data, missing or malformed stopping tables, and invalid
reaction/detector/material definitions are collected into a single
`EdECalculator::EdEConfigurationError`. Integrations should let this exception
abort selector startup rather than silently omitting a curve.

## OSCAR configuration

Enable the adapter independently in either `OSCARUP_CONF` or
`OSCARDOWN_CONF`:

```text
EDE_KINEMATIC_LINES YES #
EDE_REACTION_FILE EdECalculatorOscar.conf #
EDE_ACTIVE_MATERIAL 28-14 #
```

`DE_E_HISTOS` must also be enabled. Relative reaction filenames are resolved
under `REPLAY_CONF/CONF_PATH`. Generated objects are written below
both `OscarUp|Down/Etot_dE` and `OscarUp|Down/EdE`. Each directory contains
one canvas per telescope with the matrix, all requested reaction overlays, and
a legend. The graphs are canvas primitives rather than separate ROOT keys. The
first family uses the calculated particle energy at the reaction point; the
second uses the energy deposited in OSCAR's residual-E layer, matching the
corresponding matrix axes.

Canvas creation is deliberately deferred until all selector workers and their
`hadd` jobs have finished. Worker files contain only mergeable histograms. The
main thread then recalculates the loci once, opens each final merged ROOT file
in update mode, and writes the canvases serially. This avoids both ROOT graphics
thread-safety problems and attempts to merge already-rendered canvases.

The reaction file supports multiple blocks:

```text
REACTION Nb_2p_transfer
LAYER BACKING
TARGET 93 41
TRANSFER 2 2
GRID 0 10 0.250
# LEVELS 0.000 0.250 0.700
BRANCH BOTH
END_REACTION
```

Pure elastic scattering is configured without an excitation grid. The target
ion must match the selected target or backing material:

```text
REACTION Nb_elastic
LAYER BACKING
TARGET 93 41
ELASTIC
RESOLUTION 0.500 0.100
BRANCH HIGH
END_REACTION
```

`RESOLUTION E dE` is expressed in MeV. These values are symmetric visual
half-widths, combined in quadrature with the front-to-back reaction-depth
errors. Elastic points are drawn as fill-style-3005 error regions. Setting a
resolution to zero leaves that coordinate's physical depth uncertainty only.

`LAYER` accepts `TARGET`, `BACKING`, or the `DEGRADER` alias. Use `ELASTIC`,
`TRANSFER A Z` (material transferred from the beam to the target-like product),
or `DETECTED A Z`. `GRID minimum maximum step` produces a smooth line;
`LEVELS ...` produces points at explicit known excitation energies. Both can
be present. Excitations are specified in MeV. When `DEG_PRESENT` is `NO`, the
backing is omitted from material transport and all `LAYER BACKING` reaction
blocks are skipped, allowing one reaction file to serve both target setups.

`BRANCH HIGH` selects the higher-energy two-body solution, `BRANCH LOW`
selects the lower-energy solution, and `BRANCH BOTH` generates both from the
same reaction block. With `BOTH`, the two overlays share the reaction color
and are identified separately as `(high)` and `(low)` in the canvas legend.

Reaction colors default to the shared cyclic hex-color palette beginning with
`#2196f3`, `#f44336`, and `#3f51b5`. `LINE_COLOR` may still be supplied as an
explicit ROOT color override for an individual reaction. Smooth-grid graphs
are drawn as smoothed uncertainty bands with fill style 3005; known-level
graphs remain discrete markers.

The central point is calculated at the middle of the reaction layer. Its
asymmetric x and y errors span reactions at the entrance and exit faces. The
graph never repeats its first point, so drawing it with a line does not close
the locus as a `TCutG` would.

All required files follow the existing selector convention
`EnergyLoss/Target-<material>_Ion-<A>-<Z>.txt`. This includes the detected ion
in the target/backing and in `EDE_ACTIVE_MATERIAL`. A missing table is reported
with its full expected path and aborts selector startup.
