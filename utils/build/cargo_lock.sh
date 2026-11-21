#!/usr/bin/env bash
#set -x
cargo generate-lockfile -v --manifest-path "$MESON_DIST_ROOT/Cargo.toml"
echo "done"
