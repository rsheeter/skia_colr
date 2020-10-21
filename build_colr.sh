#!/bin/bash -x
# script version of https://gist.github.com/rsheeter/42808b2c6644736eaab0d324f58e29a9

function fetch_drott_colr() {
	pushd third_party/externals/freetype
	git remote add drott_ft_colr https://github.com/drott/freetype2-colr
	git fetch drott_ft_colr
	git checkout -b colr_v1_dag drott_ft_colr/colrV1APIGraph
	popd
}

[ -e ./bin/gn ] || ./bin/fetch-gn
[ -d third_party/externals ] || ./tools/git-sync-deps
[ "colr_v1_dag" == "$(cd third_party/externals/freetype && git rev-parse --abbrev-ref HEAD)" ] || fetch_drott_colr
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
EOF

./bin/gn gen out/Static
ninja -C out/Static colr_test

echo "Demo ready, run:"
echo "  out/Static/colr_test --help"
