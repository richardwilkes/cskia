#! /usr/bin/env bash
set -eo pipefail

# These two variables should be set in tandem to keep a consistent set of sources.
# Last set July 13, 2021 @ 8:14pm PST
DEPOT_TOOLS_COMMIT=a98084ce94230c828a03535a7fb2da1a1d04fe3b
SKIA_COMMIT=97dba836328fe54df8d19e85c51226bcefd13674

BUILD_DIR=${PWD}/skia/build
DIST=${PWD}/dist

# As changes to Skia are made, these args may need to be adjusted.
# Use 'bin/gn args $BUILD_DIR --list' to see what args are available.
COMMON_ARGS=" \
  is_debug=false \
  is_official_build=true \
  skia_enable_discrete_gpu=true \
  skia_enable_flutter_defines=false \
  skia_enable_fontmgr_android=false \
  skia_enable_fontmgr_empty=false \
  skia_enable_fontmgr_fuchsia=false \
  skia_enable_fontmgr_win_gdi=false \
  skia_enable_gpu=true \
  skia_enable_particles=true \
  skia_enable_pdf=false \
  skia_enable_skottie=false \
  skia_enable_skshaper=true \
  skia_enable_spirv_validation=false \
  skia_enable_tools=false \
  skia_enable_vulkan_debug_layers=false \
  skia_use_angle=false \
  skia_use_dawn=false \
  skia_use_dng_sdk=false \
  skia_use_egl=false \
  skia_use_expat=false \
  skia_use_experimental_xform=false \
  skia_use_ffmpeg=false \
  skia_use_fixed_gamma_text=false \
  skia_use_fontconfig=false \
  skia_use_gl=true \
  skia_use_harfbuzz=false \
  skia_use_icu=false \
  skia_use_libgifcodec=true \
  skia_use_libheif=true \
  skia_use_lua=false \
  skia_use_metal=false \
  skia_use_piex=false \
  skia_use_sfntly=false \
  skia_use_system_libjpeg_turbo=false \
  skia_use_system_libpng=false \
  skia_use_system_libwebp=false \
  skia_use_system_zlib=false \
  skia_use_vulkan=false \
  skia_use_wuffs=false \
  skia_use_xps=false \
  skia_use_zlib=true \
"

case $(uname -s) in
Darwin*)
  OS_TYPE=darwin
  SRC_LIB_NAME=${BUILD_DIR}/libskia.a
  DST_LIB_NAME=libskia_darwin.a
  export MACOSX_DEPLOYMENT_TARGET=10.12
  PLATFORM_ARGS=" \
      skia_enable_fontmgr_win=false \
      skia_use_fonthost_mac=true \
      skia_enable_fontmgr_fontconfig=false \
      skia_use_fontconfig=false \
      skia_use_freetype=false \
      skia_use_x11=false \
      extra_cflags=[ \
        \"-mmacosx-version-min=${MACOSX_DEPLOYMENT_TARGET}\" \
      ] \
      extra_cflags_cc=[ \
        \"-DHAVE_XLOCALE_H\" \
      ] \
      extra_cflags_c=[ \
        \"-DHAVE_ARC4RANDOM_BUF\", \
        \"-stdlib=libc++\" \
      ] \
    "
  ;;
Linux*)
  OS_TYPE=linux
  SRC_LIB_NAME=${BUILD_DIR}/libskia.a
  DST_LIB_NAME=libskia_linux.a
  PLATFORM_ARGS=" \
      skia_enable_fontmgr_win=false \
      skia_use_fonthost_mac=false \
      skia_enable_fontmgr_fontconfig=true \
      skia_use_fontconfig=true \
      skia_use_freetype=true \
      skia_use_x11=true \
      extra_cflags_cc=[ \
        \"-DHAVE_XLOCALE_H\" \
      ] \
      extra_cflags_c=[ \
        \"-DHAVE_ARC4RANDOM_BUF\", \
      ] \
    "
  ;;
MINGW*)
  OS_TYPE=windows
  SRC_LIB_NAME=${BUILD_DIR}/skia.dll
  DST_LIB_NAME=libskia.dll
  PYTHON_BIN=python
  export PATH="/c/python27:${PATH}"
  PLATFORM_ARGS=" \
      is_component_build=true \
      skia_enable_fontmgr_win=true \
      skia_use_fonthost_mac=false \
      skia_enable_fontmgr_fontconfig=false \
      skia_use_fontconfig=false \
      skia_use_freetype=false \
      skia_use_x11=false \
      clang_win=\"C:\\Program Files\\LLVM\" \
      extra_cflags=[ \
        \"-DSKIA_C_DLL\", \
        \"-UHAVE_NEWLOCALE\", \
        \"-UHAVE_XLOCALE_H\", \
        \"-UHAVE_UNISTD_H\", \
        \"-UHAVE_SYS_MMAN_H\", \
        \"-UHAVE_MMAP\", \
        \"-UHAVE_PTHREAD\" \
      ] \
      extra_ldflags=[ \
        \"/defaultlib:opengl32\", \
        \"/defaultlib:gdi32\" \
      ] \
    "
  ;;
*)
  echo "Unsupported OS"
  false
  ;;
esac

# Setup the Skia tree, pulling sources, if needed. For a clean build, remove the top-level
# skia directory prior to running ./build.sh
mkdir -p skia
cd skia

if [ ! -e depot_tools ]; then
  git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
  cd depot_tools
  git reset --hard ${DEPOT_TOOLS_COMMIT}
  cd ..
fi
export PATH="${PWD}/depot_tools:${PATH}"

if [ ! -e skia ]; then
  git clone https://skia.googlesource.com/skia.git
  cd skia
  git reset --hard ${SKIA_COMMIT}
  echo 'script_executable = "vpython"' >> .gn
  ${PYTHON_BIN} tools/git-sync-deps
  cd ..
fi

# Apply our changes. We get rid of the existing C api files, as they are minimal and don't
# actually provide anything useful and install our own. We also truncate the C example file
# since it relies on those old C api's.
cd skia
/bin/rm -rf src/c include/c
cp ../../capi/sk_capi.h include/
cp ../../capi/sk_capi.cpp src/
grep -v "/c/" gn/core.gni | grep -v "sk_capi.cpp" | sed -e 's@skia_core_sources = \[@&\n  "$_src/sk_capi.cpp",@' >gn/core.revised.gni
mv gn/core.revised.gni gn/core.gni
grep -v "/c/" gn/effects.gni >gn/effects.revised.gni
mv gn/effects.revised.gni gn/effects.gni
echo "int main() { return 0; }" >experimental/c-api-example/skia-c-example.c

# Perform the build
bin/gn gen $BUILD_DIR --args="${COMMON_ARGS} ${PLATFORM_ARGS}"
ninja -C $BUILD_DIR

# Copy the result into ${DIST}
mkdir -p ${DIST}/include
/bin/rm -f ${DIST}/include/*.h
cp include/sk_capi.h ${DIST}/include/
mkdir -p ${DIST}/lib
cp ${SRC_LIB_NAME} ${DIST}/lib/${DST_LIB_NAME}

cd ../..

# If present, also copy the results into the unison build tree
if [ -d ../unison ]; then
  cp ${DIST}/include/sk_capi.h ../unison/draw/include/
  cp ${DIST}/lib/${DST_LIB_NAME} ../unison/draw/${DST_LIB_NAME}
  echo "Copied distribution to unison"
fi
