#!/bin/bash -x
# script version of https://gist.github.com/rsheeter/42808b2c6644736eaab0d324f58e29a9

[ -e ./bin/gn ] || ./bin/fetch-gn
[ -d third_party/externals ] || ./tools/git-sync-deps
[ -d out/Static ] || mkdir -p out/Static

set -e

cat <<EOF > out/Static/args.gn
is_official_build = false
skia_use_fontconfig=false
skia_use_freetype=true
skia_use_system_freetype2=false
skia_enable_fontmgr_custom_directory=false
skia_enable_fontmgr_custom_embedded=false
skia_enable_fontmgr_custom_empty=true
skia_use_libjpeg_turbo_decode=false
skia_use_libjpeg_turbo_encode=false
EOF

cat out/Static/args.gn
./bin/gn gen out/Static
ninja -C out/Static colr_test

echo "Demo ready, run:"
echo "  out/Static/colr_test --help"
