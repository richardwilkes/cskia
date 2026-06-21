# C bindings for Skia (`chrome/m142`)

A flat C API (`sk_capi.h` / `sk_capi.cpp`) over [Skia](https://skia.org), built as a self-contained library for
embedding in non-C++ projects via a stable C ABI. Note that these have been tailored to the needs of my specific
projects and may not be suitable for anyone else.

[![Build](https://github.com/richardwilkes/cskia/actions/workflows/build.yml/badge.svg)](https://github.com/richardwilkes/cskia/actions/workflows/build.yml)

## Source baseline

| Component | Pin |
| --- | --- |
| [Skia](https://github.com/google/skia/tree/chrome/m142) branch | `chrome/m142` |
| [Depot Tools](https://chromium.googlesource.com/chromium/tools/depot_tools.git/+/488d7480e234ccbca59a49eadf9d23cd7aa02c31) commit | `488d7480e234ccbca59a49eadf9d23cd7aa02c31` |

Both pins are set in tandem in `build.sh` to keep a consistent set of sources.

See the file skia/skia/RELEASE_NOTES.txt for changes made to skia.

## Platforms & artifacts

`build.sh` detects the host OS and architecture and produces a single library named
`skia_<os>_<arch>`:

| OS | Architectures | Output |
| --- | --- | --- |
| macOS (`darwin`) | `amd64`, `arm64` | `libskia_darwin_<arch>.a` (static) |
| Linux | `amd64`, `arm64` | `libskia_linux_<arch>.a` (static) |
| Windows | `amd64`, `arm64` | `skia_windows_<arch>.dll` (shared) |

- macOS targets a minimum deployment of macOS 11.
- Windows is built as a component (DLL) build using LLVM/clang; symbols are exported via
  `SKIA_C_DLL` / `__declspec(dllexport)`.
- The build emits the public header `sk_capi.h` and the library into `dist/`, and (when a
  sibling `../unison` tree is present) also copies them into `unison/internal/skia`.

## Build configuration highlights

Enabled in this release:

- **PDF** document output (`skia_enable_pdf`).
- **SkShaper** text shaping (`skia_enable_skshaper`).
- **GPU**: Ganesh OpenGL backend (`skia_use_gl`, discrete GPU support).
- **Image codecs** via wuffs and bundled libjpeg-turbo / libpng / libwebp / zlib.
- **XML/SVG** support via bundled expat.
- Platform font hosting: CoreText on macOS, FreeType + fontconfig on Linux, GDI on Windows.

Deliberately disabled to keep the library lean and portable: ANGLE, Dawn, Vulkan, Metal,
EGL, HarfBuzz, ICU, DNG SDK, PIEX, Lua, Skottie, and the build tools. The corresponding
third-party dependencies are also pruned from `DEPS` before syncing.

## API surface

The C API exposes the following Skia areas (see `sk_capi.h` for full signatures):

- **Geometry & types** — points, rects, sizes, matrices, colors, image info, sampling
  options, blend/blur/clip modes.
- **Canvas** — transforms, clipping, save/restore layers, and draw operations for rects,
  ovals, arcs, circles, round rects, paths, points, lines, images (incl. nine-patch and
  rect), text, and text blobs.
- **Paint** — style, stroke parameters, color, antialias/dither, and attachment of
  shaders, color filters, mask filters, image filters, and path effects.
- **Paths & path ops** — construction (lines, curves, arcs, primitives), SVG path string
  parse/emit, transforms, queries, boolean ops, simplify, and an op builder.
- **Shaders** — solid color, blend, linear/radial/sweep/two-point conical gradients, and
  Perlin noise (fractal & turbulence).
- **Filters** — color filters (matrix, compose, high contrast, lighting, luma, blend mode),
  mask filters (blur, clip, gamma, shader, table), and a broad set of image filters
  (blur, dilate/erode, drop shadow, lighting, displacement, magnifier, matrix convolution,
  arithmetic, merge, offset, tile, image source, and more).
- **Text** — fonts, font managers/style sets, font styles, typefaces, font metrics,
  glyph/measurement helpers, and text blob building.
- **Images** — raster and encoded image creation, pixel reads, shaders from images, and
  GPU texture images.
- **Image encoding** — JPEG, PNG, and WebP encoders.
- **Surfaces** — raster surfaces, GPU backend render-target surfaces, snapshots, and
  surface properties.
- **GPU context** — Ganesh `GrDirectContext` over a native or assembled GL interface, with
  backend render targets.
- **Documents** — PDF document creation with metadata, page begin/end, and stream output.
- **Streams** — dynamic-memory and file write streams.
- **Strings & data** — `sk_string_t` and `sk_data_t` helpers.
- **Codecs** — `register_image_codecs()` to register the built-in image decoders.
