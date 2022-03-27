# C bindings for Skia

These have been tailored to the needs of my specific projects and may not be suitable for anyone else.

## Skia version

The Skia version with which these bindings have been tested is from _Wed Nov 10 21:38:56 2021 +0000_
* https://skia.googlesource.com/skia/+/6fae0523629f9abf114d8b7413f71dc7295a13e0
* https://chromium.googlesource.com/chromium/tools/depot_tools.git/+/4d3319e39c9b50747f47da35a351d428a38bcc82

**Note**: I've been unable to update the underlying Skia repository much beyond this point (roughly equivalent to the
m93 milestone). On macOS (haven't bothered to try elsewhere yet), they've changed the code in such a way that it insists
on installing its own copy of python 3.9.x during the build... but then errors out because that doesn't match version
3.8.x. I've been unable to locate what exactly is doing this -- if anyone has a clue, I'm open to suggestions, as I'd
prefer to keep matched to the latest milestone release that is no longer changing.

## Noticable changes in Skia code base reflected here

### November 10, 2021

* SkFilterQuality is gone. https://skia.googlesource.com/skia/+/aebe248575affab2137f3d3fb9f94b8e397c4986
