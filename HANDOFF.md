# HANDOFF — raytrace `world_outrun` pyramid shading

Repo: `/Users/erik/Source/external/hobbies`
Branch: `main`, clean. **No source files were modified.** All work below was reproduced
in a scratch directory; the recipe to recreate it is in "Reproduction Recipe".

## Goal

The pyramid "mountains" in `projects/raytrace/demo/world_outrun.cpp` render as totally
black silhouettes. With the object's default material they render covered in
salt-and-pepper speckle. Diagnose and fix.

Diagnosis is **complete and verified**. Nothing is fixed yet — the code changes are still
to be made, and Erik has not yet chosen which of them to apply.

Success criteria: pyramids show the neon grid material, lit, with no speckle.

## Root Cause (confirmed by measurement, not inference)

**One cause, two symptoms: the light sits at exactly the pyramid's face angle.**

`raytrace::objects::pyramid` has a fixed slope `z = h - |x| - |y|`, so every face normal is
`(±1/√3, ±1/√3, 1/√3)` — exactly 45° from horizontal.

The scene's beam at `projects/raytrace/demo/world_outrun.cpp:67` is
`raytrace::vector{0, 200, -200}` — also exactly 45°. `lights::beam::incident()` returns
`-m_distance * m_world_source`, so the vector toward the light is
`L = (0, -0.707107, +0.707107)`.

Measured per-face (probe over the real scene geometry, including the yaw):

```
F1(+x,+y) world_N=(-0.5774 +0.5774 +0.5774)  N.L=-1.4721486685148494e-17  VISIBLE
F2(-x,+y) world_N=(-0.5774 -0.5774 +0.5774)  N.L=+0.81649658092772626     hidden
F3(+x,-y) world_N=(+0.5774 +0.5774 +0.5774)  N.L=-1.4721486685148494e-17  VISIBLE
F4(-x,-y) world_N=(+0.5774 -0.5774 +0.5774)  N.L=+0.81649658092772626     hidden
```

Both camera-facing faces are exactly edge-on to the light. `N·L` is mathematically zero and
lands one rounding step **negative**.

### Symptom 1 — totally black

`projects/raytrace/source/scene.cpp:319` gates diffuse on `incident_scaling > 0.0_p`:

```cpp
color incident_light = (incident_scaling > 0.0_p) ? incident_scaling * raw_light_color : colors::black;
```

So every visible pyramid pixel loses all diffuse light. Only the ambient blend at
`scene.cpp:408` survives. In `testing/world_outrun.tga` the pyramids read RGB `(13,1,0)` —
byte-identical to the pure-black floor tiles.

Camera sweep at 2048x1080 over the real scene camera: **all 83477 pyramid pixels on both
pyramids have `N·L = -1.47e-17`**, and **0 interior holes**. The intersection code is
correct; this is purely a shading result.

### Symptom 2 — speckle

Because `L` lies *in* the face plane, the shadow ray in `scene::direct_light` travels along
the surface and grazes the pyramid's own ridge. `pyramid::collisions_along` registers that
tangential self-hit at `t = √2·y`, far above `basal::epsilon` (9.53674e-07), so the
`occluded` flag becomes a rounding coin-flip. Measured over face F1:

```
face F1: 89401 points, 48714 self-shadowed, 40687 clear
scan line oy=5.0, ox stepping by 1e-3 (S=self-shadowed, .=clear):
  ..SS.S..SS.S..SS.S..SS....S...S.S...S....SS....SS...SSS...SSS...SSS...SSS...SSSS..SSSS..SSSS..SSSS..
```

The coin-flip only reaches the image through the **specular** term, which is why the
material choice changes whether you see it:

| material | specular exponent | result |
|---|---|---|
| `mediums::grid` (inherits `opaque`, `roughness::tight`) | 100 → `pow(s,100) ≈ 2e-10` | uniform black |
| `mediums::dull` (`plain.hpp:17`, `roughness::very_loose`) | 10 → `pow(s,10) ≈ 0.1` | salt-and-pepper |

This matches Erik's own A/B (removing `.material(&mountain_grid)` → lots of speckle) and was
reproduced headlessly.

## What Worked

- **Headless renderer.** `demo_curses` is ncurses-interactive and unusable for automation.
  A ~50-line headless `main` that dlopens a world module renders 640x360 in **0.4 s**, which
  made A/B testing fast. Source and build command in "Reproduction Recipe".
- **Standalone probes linked against `install/homebrew-llvm/lib/*.a`** to measure `N·L`,
  sweep for dropped intersections, and replicate the shadow-ray occlusion test directly.
  Far more decisive than reading the shading code.
- **Verified fix:** changing `world_outrun.cpp:67` to `raytrace::vector{0, 200, -500}`
  produces fully lit, speckle-free mountains showing the neon grid. Any beam with
  `y + z < 0` works. (The dark wedge that then appears on the screen-right pyramid is the
  ellipsoid "UFO" `e0`/`e1` casting a correct shadow — not a bug.)

## What Didn't Work

- **Blaming `pyramid::collisions_along`.** It is fine. A top-down sweep of 303421 rays found
  1 miss, at exactly the apex `(0,0)`. A full camera sweep found **0 interior holes**. Do not
  re-open this; git history shows two prior rounds of pyramid intersection fixes
  (`7bfe46b`, `35995ce`) and the geometry is now solid.
- **Blaming the reflection path / `smoothness`.** Setting the pyramid material to
  `plain(white, 0.0, white, 0.0, 10.0)` — smoothness exactly 0, so no reflection ray is cast
  at `scene.cpp:380` — still speckles. The trigger is the specular exponent, not smoothness.
- **Blaming `emissive_illumination`.** `outrun::Sun` overrides `emissive()` but never sets
  `m_emissive_color`, so `medium::is_emissive()` returns false and that path never runs for it.

## Secondary Findings (independent of the main bug)

1. **`scene.cpp:326-330` adds specular without gating on `incident_scaling > 0`.** A face
   pointing away from the light still receives a highlight. This is the classic Phong bug.
   Gating it removes this whole class of speckle for every object, not just pyramids —
   worth doing even after the light is moved.
2. **`world_outrun.cpp:92-93`: `pyramid0/1.rotation(0, 0, ±90°)` is a no-op.** A 90° yaw is a
   symmetry of `z = h - |x| - |y|`. Use 45° if a different facing was intended.
3. **`pyramid::map()` (`projects/raytrace/source/objects/pyramid.cpp:109-110`) returns a
   constant `(0,0)`,** so any UV-mapped medium on a pyramid is a flat color. The grid only
   works because `mediums::grid::diffuse` (`grid.cpp:15-19`) falls back to the raw 3D point
   when no `mapper()` is set — and `mountain_grid` has none. At `scale = 20` on a
   `height = 40` pyramid that is ~2 lattice cells across, and the 3D `functions::grid`
   (`mediums/functions.cpp:208`) requires **all three** of u,v,w in-cell, so the faces come
   out almost entirely the "dark" color instead of a wireframe. This is issue **#106** (face
   UV mapping) in `TODO.md`.

## Next Steps

1. **Apply the scene fix.** Edit `projects/raytrace/demo/world_outrun.cpp:67` to
   `raytrace::vector{0, 200, -500}`. Rebuild and render; confirm the pyramids are lit with
   visible grid lines and no speckle.
2. **Ask Erik whether to also apply the engine fix** at `scene.cpp:326-330` — gate
   `specular_light` on `incident_scaling > 0.0_p`. This changes shading for *every* scene, so
   re-render the other worlds in `projects/raytrace/demo/` and eyeball for regressions before
   committing. It is the durable fix; the light change alone only dodges the degenerate angle.
3. **Ask before touching items 2 and 3 in Secondary Findings.** The `rotation()` no-op may be
   intentional/harmless; `pyramid::map()` is already tracked as GitHub issue #106.
4. **Consider a regression test** in `projects/raytrace/test/gtest_pyramid.cpp`: assert that a
   light exactly in a face plane does not produce a self-shadow hit. There is no test today
   that covers grazing self-intersection.
5. **`GOTCHAS.md` does not exist in this repo.** Erik was offered one seeded with the
   45°-slope trap. Create it if he says yes.

### Branching

Per `~/.claude/CLAUDE.md`, this repo is tracked by GitHub issues, not Jira. There is no
existing issue for this bug — file one, or use a `fix/` branch. Do not invent a Jira key.

## Reproduction Recipe

Everything below assumes cwd `/Users/erik/Source/external/hobbies` and an existing
`install/homebrew-llvm/` tree (produced by `./testing.sh` or
`cmake --build build/homebrew-llvm --target install`).

### Headless renderer

`demo_curses` cannot be scripted. Write this to a scratch file (it is not in the repo):

```cpp
// headless.cpp — loads a world module and renders once, no ncurses.
#include <basal/module.hpp>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <raytrace/raytrace.hpp>
#include <string>
#include "world.hpp"

using namespace basal::literals;
using namespace geometry::operators;

int main(int argc, char* argv[]) {
    if (argc < 6) {
        printf("usage: %s <module.dylib> <out.tga> <w> <h> <subsamples> [reflections] [fov]\n", argv[0]);
        return 1;
    }
    size_t width = (size_t)atoi(argv[3]), height = (size_t)atoi(argv[4]);
    size_t subsamples = (size_t)atoi(argv[5]);
    size_t reflections = (argc > 6) ? (size_t)atoi(argv[6]) : 4u;
    precision fov = (argc > 7) ? (precision)atof(argv[7]) : 55.0_p;

    basal::module mod(argv[1]);
    if (not mod.is_loaded()) { printf("failed to load\n"); return 2; }
    auto get_world = mod.get_symbol<raytrace::world_getter>("get_world");
    if (get_world == nullptr) { printf("no get_world symbol\n"); return 3; }
    raytrace::world& world = *get_world();

    raytrace::scene scene;
    raytrace::camera view(height, width, iso::degrees(fov));
    raytrace::vector looking = (world.looking_at() - world.looking_from()).normalized();
    view.move_to(world.looking_from(), world.looking_from() + looking);
    scene.set_background_mapper(std::bind(&raytrace::world::background, &world, std::placeholders::_1));
    world.add_to(scene);
    scene.set_ambient_light(world.ambient());
    scene.render(view, argv[2], subsamples, reflections, std::nullopt, raytrace::image::AAA_MASK_DISABLED);
    return 0;
}
```

### Build command (works for the renderer, the probes, and world `.dylib` variants)

Add `-shared -fPIC` when building a world module. Add `-Iprojects/raytrace/demo` when the
source includes `world.hpp`.

```bash
/Users/erik/.local/share/hb-alternatives/bin/clang++ -std=c++20 -stdlib=libc++ -O2 -g \
 -Iprojects/raytrace/demo \
 -Ibuild/homebrew-llvm/projects/raytrace/include -Iprojects/raytrace/include \
 -Ibuild/homebrew-llvm/projects/linalg/include -Iprojects/linalg/include \
 -Ibuild/homebrew-llvm/projects/basal/include -Iprojects/basal/include \
 -Ibuild/homebrew-llvm/projects/units_of_measure/include -Iprojects/units_of_measure/include \
 -Ibuild/homebrew-llvm/projects/fourcc/include -Iprojects/fourcc/include \
 -Ibuild/homebrew-llvm/projects/noise/include -Iprojects/noise/include \
 -Ibuild/homebrew-llvm/projects/geometry/include -Iprojects/geometry/include \
 -isystem /opt/homebrew/opt/libomp/include -fopenmp -L/opt/homebrew/opt/libomp/lib \
 <source.cpp> -o <output> \
 install/homebrew-llvm/lib/libhobbies-raytrace.a \
 install/homebrew-llvm/lib/libhobbies-geometry.a \
 install/homebrew-llvm/lib/libhobbies-linalg.a \
 install/homebrew-llvm/lib/libhobbies-fourcc.a \
 install/homebrew-llvm/lib/libhobbies-noise.a \
 install/homebrew-llvm/lib/libhobbies-uom.a \
 install/homebrew-llvm/lib/libhobbies-basal.a
```

Notes:
- `-L/opt/homebrew/opt/libomp/lib` is required; `-fopenmp` alone gives `ld: library 'omp' not found`.
- Ignore the `built for newer 'macOS' version` linker warnings.
- A/B a scene by copying `projects/raytrace/demo/world_outrun.cpp` to scratch, editing the
  copy, building it as a `.dylib`, and passing that to the headless renderer. This keeps the
  repo clean.

### Render and view

```bash
./headless <world.dylib> out.tga 640 360 1 4 55
sips -s format png out.tga --out out.png
```

`testing/world_outrun.tga` (2048x1080) is the untracked "before" image showing the black
pyramids. Read raw TGA pixels with a short Python script — TGA type 2, 24bpp BGR, header
18 bytes, bottom-left origin when `desc` bit 5 is clear. No PIL is installed.

### Probes that produced the numbers above

Three short `main()` programs, built with the same command (no `-shared`):

1. **`N·L` per face** — construct `pyramid{point{50,-100,0}, 40.0_p}` with yaw 90°, set
   `L = vector{{0,-200,200}}.normalized()`, then print `dot(L, p.normal(p.forward_transform(objpt)))`
   for one point in each object-space quadrant. Also print `dot(V, N)` to mark which faces
   are visible from `point{0,50,10}`.
2. **Camera sweep** — rebuild the `OutrunWorld` camera basis (`from{0,50,10}`, `at{0,0,10}`,
   fov 55°, 2048x1080), cast a ray per pixel at both pyramids, record a hit mask, then count
   miss pixels whose 4 neighbours all hit (interior holes) and bucket `N·L` by sign.
3. **Self-shadow** — for a fine grid of object-space points on face F1, build
   `ray{p.forward_transform(objpt), L}`, reverse-transform it, call
   `p.collisions_along()`, and apply the same filters `scene::direct_light` uses
   (`is_nan`, `normal.is_zero()`, `distance < basal::epsilon`). Count how many points find a
   blocker. Print one scan line as `S`/`.` to see the alternation.
