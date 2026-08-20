# Chroma Replace — Algorithm Recommendations

Review of `demo/main_chroma_replace.cpp` and the `source/chroma_replace.cpp` /
`source/matting.cpp` implementations, identifying additional algorithms that could
improve the chroma replace feature.

## Current State

The `--type` option already selects between seven algorithms:

| `--type`  | Family          | Description |
|-----------|-----------------|-------------|
| `vlahos`  | per-pixel keying | `alpha = key channel - max(other channels)`, clamped to [0,1] |
| `mishima` | per-pixel keying | Two-state hidden Markov model estimated with forward-backward along each scanline |
| `closedform` | matting solver | Levin, Lischinski & Weiss 2008, matting Laplacian solved with PCG over a rough trimap |
| `bayesian` | matting solver | Chuang et al. 2001, per-pixel Bayesian color-line estimate |
| `knn`     | matting solver | Chen, Li & Tang 2013, nonlocal KNN affinities solved as a graph Laplacian |
| `global`  | matting solver | Aksoy et al. 2017 "Information Flow" via OpenCV alphamat (available on OpenCV 4.7+/5.x) |
| `fused`   | keying + matting | Keying-derived trimap (green excess) pins definite subject, closed-form solver soft-keys the spill band |

Supporting infrastructure already present:

- Clip black/white remapping (Keylight style), `--clip-black` / `--clip-white`
- Protect rectangles and auto-protect via Apple Vision (`--protect`, `--protect-auto`),
  with feathering (`--protect-feather`)
- HSV trimap (`matting::build_trimap`) and keying-derived trimap
  (`matting::build_trimap_from_keying`)
- Frame scaling, still-frame extraction, image/video output

The pipeline (`compute alpha` → `remap alpha` → composite) is clean and trivially
extensible: new algorithms slot in as new `ChromaType` values or new post-steps.

## Gaps and Additional Algorithms

### 1. Spill suppression / despill — *top priority*

**Problem:** Nothing despills the foreground. Real footage leaves green fringe on
hair, shoulders, and reflective objects — every algorithm above keys the screen but
keeps the green cast on the subject.

**Algorithm:** Industry keyers (Ultimatte, Keylight) run a separate despill pass:
where foreground alpha is low/medium, clamp the key channel toward the mean of the
other two, weighted by alpha. Concretely, `despill(image, alpha, key)` applied after
`compute_alpha()` and before compositing.

**Fit:** New `--despill` flag; the function operates on the keyed-out foreground so it
composes into `key_out()` naturally (spill suppression is then baked into the output
even without a background).

**Effort/impact:** ~40 lines, very high quality impact for real footage. Highest
quality-per-effort item on this list.

### 2. Screen color estimation — *top priority*

**Problem:** The key is a fixed named color, e.g. `(0,255,0)`. Real screens are
gradient, vignetted, unevenly lit, and off-hue; keying against a pure named color is
why shadows and corners need clip-into-submission.

**Algorithms, in increasing power:**

- **Global per-frame estimate:** histogram-peak color near the key hue (or the mean
  over pixels whose green excess > `bg_keep`), replacing the pure named color.
  Add as `--screen-estimate auto`.
- **Per-pixel local screen model:** smooth / inpaint the regions classified as screen
  to estimate a *local* screen color at every pixel, then key against that local
  color. This is the "screen gain" idea behind Keylight's screen balance and fixes
  shadowed/vignetted screens without hard clipping.
- **Temporal (video):** EMA or median of the screen estimate across frames — cheap
  deflicker for the screen regions.

**Fit:** New option(s); per-pixel variant feeds `compute_alpha` with a per-pixel key
instead of a scalar `cv::Vec3b`.

**Effort/impact:** Global estimate is small; local model is medium. Very high impact
for unevenly lit footage.

### 3. Keylight-style keyer — new `ChromaType`

**Algorithm:** Nuke/AE's industry standard. Core matte from luminance-scaled
hue/channel match with softness, plus screen balance and despill. Complements the
existing clip black/white remapping directly and addresses weakly-keyed shadows.

**Fit:** New `ChromaType::Keylight` behind `--type keylight`; interacts naturally with
recommendation 2 (screen balance is derived from the estimated screen color).

**Effort/impact:** Medium. A first-class algorithm alongside Vlahos/Mishima.

### 4. Guided-filter alpha refinement — post-step

**Algorithm:** After any matte, run a guided filter guided by the color frame to snap
matte edges to image edges and soften hard Vlahos/Mishima edges. O(n) via box filters,
a standard post-step in matting pipelines.

**Fit:** New `--refine <radius>` flag applying to every `--type`.

**Effort/impact:** Small; improves edge quality across all algorithms.

### 5. Shared Sampling Matting (Gastal & Oliveira 2010)

**Algorithm:** Linear-time matting that collects color samples from known regions and
solves alpha per pixel without a global solver iteration loop. Closed-form's 256 CG
iterations is the slowest path for video; shared sampling is roughly real-time friendly.

**Fit:** New `ChromaType` behind `--type shared`, reusing `build_trimap` /
`build_trimap_from_keying` unchanged.

**Effort/impact:** Medium. Best value when per-frame cost matters (video workflows).

### 6. Vision-segmentation-guided matting

**Algorithm:** `source/subject_mask.mm` already runs
`VNGenerateForegroundInstanceMaskRequest` and can produce a *float confidence mask*
(`kCVPixelFormatType_OneComponent32Float`), which is currently thresholded at 0.5.
Feeding that soft mask in as a foreground prior — dilate/erode it to form the unknown
band, then solve in-band with a matting solver — gives the classic segmentation +
matting pipeline without adding a neural net to the project.

**Fit:** New matting mode; the existing protect/feather plumbing already provides most
of the machinery.

**Effort/impact:** Medium-large (Objective-C++ + new solver path). High quality for
person shots; the native Vision model is already linked in.

### 7. Minor polish

- **Trimap morphological cleanup:** erode/dilate the trimap before solving to remove
  speckle (`--trimap-clean`). Trivial.
- **Temporal matte smoothing (video):** EMA the output alpha across frames to kill
  matte flicker independent of screen estimation.

## Recommendation Summary

| Item | Impact | Effort | Suggested order |
|------|--------|--------|-----------------|
| 1. Despill | very high | ~40 lines | 1 |
| 2. Screen estimation | very high | small–medium | 1 |
| 4. Guided refinement | high | small | 2 |
| 3. Keylight-style keyer | high | medium | 2 |
| 5. Shared sampling matting | medium (speed) | medium | 3 |
| 6. Vision-guided matting | high | medium–large | 3 |
| 7. Trimap cleanup / temporal smoothing | low–medium | trivial–small | anytime |

The biggest quality jump per unit of effort is items **1 + 2 + 4** (despill +
adaptive screen estimation + guided alpha refinement): they fix the things real
footage breaks and reuse the existing pipeline. Then **3** (Keylight-style keyer) as a
first-class algorithm. Items 5 and 6 are valuable but larger; 7 is polish.

## Implementation Notes

- New algorithms go in `source/chroma_replace.cpp` (keying/despill) or
  `source/matting.cpp` (solvers/trimaps), with declarations in the matching headers.
- All `ChromaType` additions must be added to `parse_chroma_type()` and its accepted
  name aliases.
- Per AGENTS.md, every new function needs gtest coverage in
  `projects/vision/test/gtest_vision.cpp`, with mocks in `test/mocks` for abstract
  interfaces.
- OpenCV 5.0.0 is installed (alphamat available), so `global_matting()` is usable; no
  OpenCV changes are required for any of the above.