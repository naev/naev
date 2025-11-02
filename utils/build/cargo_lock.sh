#!/usr/bin/env bash
#set -x
# Would be trivial with --lockfile-path but its unstable...
cargo generate-lockfile -v --manifest-path "$1"
cp "$MESON_BUILD_ROOT/Cargo.toml" "$MESON_DIST_ROOT/Cargo.toml"
echo "done"
