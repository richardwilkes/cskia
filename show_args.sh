#! /usr/bin/env bash
set -eo pipefail

export PATH="${PWD}/skia/depot_tools:${PATH}"

cd skia/skia
bin/gn args ../build --list
