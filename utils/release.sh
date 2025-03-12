#!/usr/bin/env bash
# =============================================================================
# This script orchestrates builds for various targets (source, linux, windows, macos)
# using Docker. It pulls required images, sets up ccache directories, and launches
# containerized build environments with the necessary privileges.
#
# TODO: Integrate the proper entrypoint so that advanced environment setup is performed,
#       and fix macos builds (they rely on the entrypoint script).
# =============================================================================

set -e

usage() {
    echo "usage: $(basename "$0") -s <SOURCEROOT> -b <BUILDROOT> -t <TARGETS>"
    echo "TARGETS: comma-separated list of: source, linux, windows, macos or 'all'"
    exit 1
}

# Defaults
TARGETS=""

while getopts s:b:t: OPTION; do
    case $OPTION in
    s)
        SOURCEROOT="${OPTARG}"
        ;;
    b)
        BUILDPATH="${OPTARG}"
        ;;
    t)
        TARGETS="${OPTARG}"
        ;;
    *)
        usage
        ;;
    esac
done

if [ -z "$SOURCEROOT" ] || [ -z "$BUILDPATH" ] || [ -z "$TARGETS" ]; then
    usage
fi

mkdir -p "$BUILDPATH"

# If TARGETS is 'all', then set to all targets.
if [ "$TARGETS" = "all" ]; then
    TARGETS="source,linux,windows"
fi

IFS=',' read -r -a target_array <<< "$TARGETS"
# New: Prompt to pull only the required docker images for the selected targets.
read -r -p "Would you like to pull the required docker images? [Y/n] " answer
if [[ "$answer" =~ ^[Yy]|^$ ]]; then
    declare -A uniqueImages
    for target in "${target_array[@]}"; do
        case "$target" in
            source)
                uniqueImages["ghcr.io/naev/naev-release:latest"]=1
                ;;
            linux)
                uniqueImages["ghcr.io/naev/naev-steamruntime:latest"]=1
                ;;
            windows)
                uniqueImages["ghcr.io/naev/naev-windows:latest"]=1
                ;;
            macos)
                uniqueImages["ghcr.io/naev/naev-macos:latest"]=1
                ;;
        esac
    done
    for img in "${!uniqueImages[@]}"; do
        echo "Pulling $img..."
        docker pull "$img"
    done
fi

for target in "${target_array[@]}"; do
    TARGET_BUILD="${BUILDPATH}/${target}"
    mkdir -p "$TARGET_BUILD"
    mkdir -p "$BUILDPATH"/output
    case "$target" in
        source)
            echo "Building source tarball..."
            docker run --rm -u "$(id -u):$(id -g)" \
                -v "$SOURCEROOT":"${HOME}"/source:Z \
                -v "$TARGET_BUILD":"${HOME}"/build:Z \
                -e CCACHE_DIR="${HOME}"/build/ccache -e CCACHE_TMPDIR="${HOME}"/build/ccache/tmp \
                ghcr.io/naev/naev-release:latest \
                bash -c "meson setup ${HOME}/build ${HOME}/source -Dexecutable=disabled -Ddocs_c=disabled -Ddocs_lua=disabled && meson dist -C ${HOME}/build --allow-dirty --no-tests --include-subprojects"
            cp -r "$TARGET_BUILD"/meson-dist/naev-*.tar.xz "$BUILDPATH"/output
            ;;
        linux)
            echo "Building Linux AppImage and generating zsync..."
            docker run --rm -u "$(id -u):$(id -g)" \
                -v "$SOURCEROOT":"${HOME}"/source:Z \
                -v "$TARGET_BUILD":"${HOME}"/build:Z \
                -e CCACHE_DIR="${HOME}"/build/ccache -e CCACHE_TMPDIR="${HOME}"/build/ccache/tmp \
                ghcr.io/naev/naev-steamruntime:latest \
                bash -c "${HOME}/source/utils/buildAppImage.sh -i -d -s ${HOME}/source -b ${HOME}/build"
            cp -r "$TARGET_BUILD"/dist/* "$BUILDPATH"/output
            ;;
        windows)
            echo "Building Windows installer..."
            docker run --rm -u "$(id -u):$(id -g)" \
                -v "$SOURCEROOT":"${HOME}"/source:Z \
                -v "$TARGET_BUILD":"${HOME}"/build:Z \
                -e CCACHE_DIR="${HOME}"/build/ccache -e CCACHE_TMPDIR="${HOME}"/build/ccache/tmp \
                ghcr.io/naev/naev-windows:latest \
                bash -c "meson setup ${HOME}/build ${HOME}/source --prefix=\"${HOME}/build/windows\" --bindir=. -Dndata_path=. --cross-file='${HOME}/source/utils/build/windows_cross_mingw_ucrt64.ini' --buildtype=debug --force-fallback-for=glpk,SuiteSparse -Dinstaller=true -Drelease=true -Db_lto=false -Dauto_features=enabled -Ddocs_c=disabled -Ddocs_lua=disabled && meson compile -C ${HOME}/build && meson install -C ${HOME}/build"
            cp -r "$TARGET_BUILD"/dist/* "$BUILDPATH"/output
            ;;
        macos)
            echo "Building macOS universal DMG..."
            # Build x86_64 target:
            docker run --rm -u "$(id -u):$(id -g)" \
                -v "$SOURCEROOT":"${HOME}"/source:Z \
                -v "$TARGET_BUILD":"${HOME}"/build:Z \
                -e CCACHE_DIR="${HOME}"/build/ccache -e CCACHE_TMPDIR="${HOME}"/build/ccache/tmp \
                ghcr.io/naev/naev-macos:latest \
                bash -c "mkdir -p ${HOME}/build/macos/x86_64 && mkdir -p ${HOME}/build/macos/x86_build && meson setup ${HOME}/build/macos/x86_build ${HOME}/source --prefix=\"${HOME}/build/macos/Naev_x86.app\" --bindir=Contents/MacOS -Dndata_path=Contents/Resources --cross-file='${HOME}/source/utils/build/macos_cross_osxcross.ini' --buildtype=debug -Dinstaller=false -Drelease=true -Db_lto=true -Dauto_features=enabled -Ddocs_c=disabled -Ddocs_lua=disabled && meson compile -C ${HOME}/build/macos/x86_build && meson install -C ${HOME}/build/macos/x86_build && cp -r ${HOME}/build/macos/Naev_x86.app ${HOME}/build/macos/x86_64"
            # Build arm64 target:
            docker run --rm -u "$(id -u):$(id -g)" \
                -v "$SOURCEROOT":"${HOME}"/source:Z \
                -v "$TARGET_BUILD":"${HOME}"/build:Z \
                -e CCACHE_DIR="${HOME}"/build/ccache -e CCACHE_TMPDIR="${HOME}"/build/ccache/tmp \
                ghcr.io/naev/naev-macos:latest \
                bash -c "mkdir -p ${HOME}/build/macos/arm64 && mkdir -p ${HOME}/build/macos/arm_build && meson setup ${HOME}/build/macos/arm_build ${HOME}/source --prefix=\"${HOME}/build/macos/Naev_arm.app\" --bindir=Contents/MacOS -Dndata_path=Contents/Resources --cross-file='${HOME}/source/utils/build/macos_aarch64_cross_osxcross.ini' --buildtype=debug -Dinstaller=false -Drelease=true -Db_lto=true -Dauto_features=enabled -Ddocs_c=disabled -Ddocs_lua=disabled && meson compile -C ${HOME}/build/macos/arm_build && meson install -C ${HOME}/build/macos/arm_build && cp -r ${HOME}/build/macos/Naev_arm.app ${HOME}/build/macos/arm64"
            # Package universal bundle:
            docker run --rm -u "$(id -u):$(id -g)" \
                -v "$SOURCEROOT":"${HOME}"/source:Z \
                -v "$TARGET_BUILD":"${HOME}"/build:Z \
                -e CCACHE_DIR="${HOME}"/build/ccache -e CCACHE_TMPDIR="${HOME}"/build/ccache/tmp \
                ghcr.io/naev/naev-macos:latest \
                bash -c "${HOME}/source/utils/buildUniversalBundle.sh -d -i ${HOME}/source/extras/macos/dmg_assets -e ${HOME}/source/extras/macos/entitlements.plist -a ${HOME}/build/macos/arm64/Naev.app -x ${HOME}/build/macos/x86_64/Naev.app -b ${HOME}/build/macos"
            ;;
        *)
            echo "Unknown target: $target"
            ;;
    esac
done
