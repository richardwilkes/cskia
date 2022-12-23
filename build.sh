#! /usr/bin/env bash
set -eo pipefail

# These two variables should be set in tandem to keep a consistent set of sources.
# Last set Sat Dec 17 10:01:14 PDT 2022
DEPOT_TOOLS_COMMIT=5decb175432cb284b6f8ee102dc1b908b58d8e41
SKIA_BRANCH=chrome/m110

for arg in "$@"
do
  case "$arg" in
    --args|-a) SHOW_ARGS=1 ;;
    --clean|-c) CLEAN=restore ;;
    --CLEAN|-C) CLEAN=full ;;
    --help|-h)
      echo "$0 [options]"
      echo "  -a, --args  Display the available args list for the skia build (no build)"
      echo "  -c, --clean Remove the dist and skia/build directories (no build)"
      echo "  -C, --CLEAN Remove the dist and skia directories (no build)"
      echo "  -h, --help This help text"
      exit 0
      ;;
    *)
      echo "Invalid argument: $arg"
      exit 1
      ;;
  esac
done

if [ "$CLEAN"x == "fullx" ]; then
  /bin/rm -rf dist skia
  exit 0
fi

if [ "$CLEAN"x == "restorex" ]; then
  /bin/rm -rf dist skia/build
  if [ -d skia/skia ]; then
    cd skia/skia
    git checkout -- .
    rm include/sk_capi.h src/sk_capi.cpp
  fi
  exit 0
fi

if [ "$SHOW_ARGS"x == "1x" ]; then
  export PATH="${PWD}/skia/depot_tools:${PATH}"
  cd skia/skia
  bin/gn args ../build --list --short
  exit 0
fi

BUILD_DIR=${PWD}/skia/build
DIST=${PWD}/dist

# As changes to Skia are made, these args may need to be adjusted.
# Use 'bin/gn args $BUILD_DIR --list' to see what args are available.
# Note: Turned off skia_use_libheif since having it on did not allow .heif files to be loaded...
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
  skia_enable_pdf=true \
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
  skia_use_ffmpeg=false \
  skia_use_fixed_gamma_text=false \
  skia_use_fontconfig=false \
  skia_use_gl=true \
  skia_use_harfbuzz=false \
  skia_use_icu=false \
  skia_use_libheif=false \
  skia_use_lua=false \
  skia_use_metal=false \
  skia_use_piex=false \
  skia_use_sfntly=false \
  skia_use_system_libjpeg_turbo=false \
  skia_use_system_libpng=false \
  skia_use_system_libwebp=false \
  skia_use_system_zlib=false \
  skia_use_vulkan=false \
  skia_use_wuffs=true \
  skia_use_xps=false \
  skia_use_zlib=true \
"

case $(uname -s) in
Darwin*)
  OS_TYPE=darwin
  LIB_NAME=libskia.a
  case $(uname -m) in
  x86_64*)
    UNISON_LIB_NAME=libskia_darwin_amd64.a
    export MACOSX_DEPLOYMENT_TARGET=10.14
    ;;
  arm*)
    UNISON_LIB_NAME=libskia_darwin_arm64.a
    export MACOSX_DEPLOYMENT_TARGET=11
    ;;
  esac
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
  LIB_NAME=libskia.a
  UNISON_LIB_NAME=libskia_linux.a
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
  LIB_NAME=skia.dll
  UNISON_LIB_NAME=skia_windows.dll
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

# Setup the Skia tree, pulling sources, if needed.
mkdir -p skia
cd skia

if [ ! -e depot_tools ]; then
  git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
  cd depot_tools
  git reset --hard "${DEPOT_TOOLS_COMMIT}"
  cd ..
fi
export PATH="${PWD}/depot_tools:${PATH}"

if [ ! -e skia ]; then
  git clone https://github.com/google/skia.git
  cd skia
  git checkout "${SKIA_BRANCH}"
  python3 tools/git-sync-deps
  cd ..
fi

# Apply our changes.
cd skia
/bin/rm -rf src/c include/c
cp ../../capi/sk_capi.h include/
cp ../../capi/sk_capi.cpp src/
grep -v src/sk_capi.cpp gn/core.gni | sed -e 's@skia_core_sources = \[@&\
  "$_src/sk_capi.cpp",@' > gn/core.gni.new
/bin/mv gn/core.gni.new gn/core.gni

# Perform the build
bin/gn gen "${BUILD_DIR}" --args="${COMMON_ARGS} ${PLATFORM_ARGS}"
ninja -C "${BUILD_DIR}"

# Copy the result into ${DIST}
mkdir -p "${DIST}/include"
/bin/rm -f ${DIST}/include/*.h
cp include/sk_capi.h "${DIST}/include/"
mkdir -p "${DIST}/lib/${OS_TYPE}"
cp "${BUILD_DIR}/${LIB_NAME}" "${DIST}/lib/${OS_TYPE}/"

cd ../..

# If present, also copy the results into the unison build tree
if [ -d ../unison ]; then
  RELATIVE_UNISON_DIR=../unison/internal/skia
  mkdir -p "${RELATIVE_UNISON_DIR}"
  cp "${DIST}/include/sk_capi.h" "${RELATIVE_UNISON_DIR}/"
  cp "${DIST}/lib/${OS_TYPE}/${LIB_NAME}" "${RELATIVE_UNISON_DIR}/${UNISON_LIB_NAME}"
  echo "Copied distribution to unison"
fi
