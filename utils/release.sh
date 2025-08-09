#!/usr/bin/env bash
# =============================================================================
# This script orchestrates builds for various targets (source, linux, windows, macos)
# using Docker. It pulls required docker images (or uses local images when specified),
# sets up ccache directories, and launches containerized build environments.
#
# Build outputs and temporary files are organized under the specified build root directory.
#
# Parameters:
#   -s <SOURCEROOT> : Path to the source code directory.
#   -b <BUILDROOT>  : Directory to store build artifacts and outputs.
#   -t <TARGETS>    : Comma-separated list of targets to build; valid targets include
#                     "source", "linux", "windows", and "macos". Alternatively, "all" to
#                     build all targets.
# Optional:
#   -d              : Enable debug mode (bash -x) for detailed command output.
#
# Additional Environment Variable:
#   USE_LOCAL       : When set to "1", the script uses local Docker images (e.g., 'naev-steamruntime')
#                     instead of pulling from the GitHub Container Registry.
#
# TODO:
#   - Integrate a proper entrypoint for advanced environment configuration.
#   - Fix macos builds which currently depend on an external entrypoint script.
# =============================================================================

set -e

usage() {
    echo "Usage: $(basename "$0") -s <SOURCEROOT> -b <BUILDROOT> -t <TARGETS> [-d]"
    echo
    echo "Parameters:"
    echo "  -s   Path to the source directory containing the code."
    echo "  -b   Path to the build directory where artifacts and outputs will be stored."
    echo "  -t   Comma-separated list of build targets. Valid options are:"
    echo "         source   : Build the source tarball."
    echo "         linux    : Build the Linux AppImage and generate zsync."
    echo "         windows  : Build the Windows installer."
    echo "         macos    : Build the macOS universal DMG (requires additional fixes)."
    echo "         all      : Build all available targets."
    echo "  -d   Enable debug mode (bash -x) for detailed command output."
    echo
    echo "Example:"
    echo "  $(basename "$0") -s /path/to/source -b /path/to/build -t linux,windows -d"
    exit 1
}

# Defaults
TARGETS=""

while getopts s:b:t:d OPTION; do
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
    d)
        set -x
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
    TARGETS="source,linux,windows,macos"
fi

IFS=',' read -r -a target_array <<< "$TARGETS"

# Determine docker image names based on USE_LOCAL env variable
if [ "$USE_LOCAL" = "1" ]; then
    image_source="naev-release"
    image_linux="naev-steamruntime"
    image_windows="naev-windows"
    image_macos="naev-macos"
    echo "Using local docker images"
else
    image_source="ghcr.io/naev/naev-release:latest"
    image_linux="ghcr.io/naev/naev-steamruntime:latest"
    image_windows="ghcr.io/naev/naev-windows:latest"
    image_macos="ghcr.io/naev/naev-macos:latest"
fi

# Update image pulling block
if [ "$USE_LOCAL" != "1" ]; then
    read -r -p "Would you like to pull the required docker images? [Y/n] " answer
    if [[ "$answer" =~ ^[Yy]|^$ ]]; then
        declare -A uniqueImages
        for target in "${target_array[@]}"; do
            case "$target" in
                source)
                    uniqueImages["$image_source"]=1
                    ;;
                linux)
                    uniqueImages["$image_linux"]=1
                    ;;
                windows)
                    uniqueImages["$image_windows"]=1
                    ;;
                macos)
                    uniqueImages["$image_macos"]=1
                    ;;
            esac
        done
        for img in "${!uniqueImages[@]}"; do
            echo "Pulling $img..."
            docker pull "$img"
        done
    fi
else
    echo "Skipping docker image pull; using local images."
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
                "$image_source" \
                bash -c "meson setup ${HOME}/build ${HOME}/source -Dexecutable=disabled -Ddocs_c=disabled -Ddocs_lua=disabled && meson dist -C ${HOME}/build --allow-dirty --no-tests --include-subprojects"
            cp -r "$TARGET_BUILD"/meson-dist/naev-*.tar.xz "$BUILDPATH"/output
            ;;
        linux)
            echo "Building Linux AppImage and generating zsync..."
            docker run --rm -u "$(id -u):$(id -g)" \
                -v "$SOURCEROOT":"${HOME}"/source:Z \
                -v "$TARGET_BUILD":"${HOME}"/build:Z \
                -e CCACHE_DIR="${HOME}"/build/ccache -e CCACHE_TMPDIR="${HOME}"/build/ccache/tmp \
                "$image_linux" \
                bash -c "${HOME}/source/utils/buildAppImage.sh -i -d -s ${HOME}/source -b ${HOME}/build"
            cp -r "$TARGET_BUILD"/dist/* "$BUILDPATH"/output
            ;;
        windows)
            echo "Building Windows installer..."
            docker run --rm -u "$(id -u):$(id -g)" \
                -v "$SOURCEROOT":"${HOME}"/source:Z \
                -v "$TARGET_BUILD":"${HOME}"/build:Z \
                -e CCACHE_DIR="${HOME}"/build/ccache -e CCACHE_TMPDIR="${HOME}"/build/ccache/tmp \
                "$image_windows" \
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
                "$image_macos" \
                bash -c "mkdir -p ${HOME}/build/macos/x86_64 && mkdir -p ${HOME}/build/macos/x86_build && meson setup ${HOME}/build/macos/x86_build ${HOME}/source --prefix=\"${HOME}/build/macos/Naev_x86.app\" --bindir=Contents/MacOS -Dndata_path=Contents/Resources --cross-file='${HOME}/source/utils/build/macos_cross_osxcross.ini' --buildtype=debug -Dinstaller=false -Drelease=true -Db_lto=true -Dauto_features=enabled -Ddocs_c=disabled -Ddocs_lua=disabled && meson compile -C ${HOME}/build/macos/x86_build && meson install -C ${HOME}/build/macos/x86_build && cp -r ${HOME}/build/macos/Naev_x86.app ${HOME}/build/macos/x86_64"
            # Build arm64 target:
            docker run --rm -u "$(id -u):$(id -g)" \
                -v "$SOURCEROOT":"${HOME}"/source:Z \
                -v "$TARGET_BUILD":"${HOME}"/build:Z \
                -e CCACHE_DIR="${HOME}"/build/ccache -e CCACHE_TMPDIR="${HOME}"/build/ccache/tmp \
                "$image_macos" \
                bash -c "mkdir -p ${HOME}/build/macos/arm64 && mkdir -p ${HOME}/build/macos/arm_build && meson setup ${HOME}/build/macos/arm_build ${HOME}/source --prefix=\"${HOME}/build/macos/Naev_arm.app\" --bindir=Contents/MacOS -Dndata_path=Contents/Resources --cross-file='${HOME}/source/utils/build/macos_aarch64_cross_osxcross.ini' --buildtype=debug -Dinstaller=false -Drelease=true -Db_lto=true -Dauto_features=enabled -Ddocs_c=disabled -Ddocs_lua=disabled && meson compile -C ${HOME}/build/macos/arm_build && meson install -C ${HOME}/build/macos/arm_build && cp -r ${HOME}/build/macos/Naev_arm.app ${HOME}/build/macos/arm64"
            # Package universal bundle:
            docker run --rm -u "$(id -u):$(id -g)" \
                -v "$SOURCEROOT":"${HOME}"/source:Z \
                -v "$TARGET_BUILD":"${HOME}"/build:Z \
                -e CCACHE_DIR="${HOME}"/build/ccache -e CCACHE_TMPDIR="${HOME}"/build/ccache/tmp \
                "$image_macos" \
                bash -c "${HOME}/source/utils/buildUniversalBundle.sh -d -i ${HOME}/source/extras/macos/dmg_assets -e ${HOME}/source/extras/macos/entitlements.plist -a ${HOME}/build/macos/arm64/Naev.app -x ${HOME}/build/macos/x86_64/Naev.app -b ${HOME}/build/macos"
            ;;
        *)
            echo "Unknown target: $target"
            ;;
    esac
done
