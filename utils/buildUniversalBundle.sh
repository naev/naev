#!/bin/bash

# macOS Universal Bundle BUILD SCRIPT FOR NAEV
#
# Pass in [-d] -i <DMGASSETSPATH> (Override location of dmg_assets folder) -e <ENTITLEMENTPLISTPATH> (Override location of entitlements.plist file) -a <ARM64BUNDLEPATH> (Override location of ARM64 Naev.app bundle path) -x <X8664BUNDLEPATH> (Override location of x86_64 Naev.app bundle path) -b <BUILDPATH> (Override location of build directory if wanted)

# Output destination is ${WORKPATH}/dist

set -e

# Defaults
ARM64BUNDLEPATH="$(pwd)/arm64"
X8664BUNDLEPATH="$(pwd)/x86_64"
ENTITLEMENTPLISTPATH="$(pwd)/entitlements.plist"
DMGASSETSPATH="$(pwd)/dmg_assets"

while getopts di:e:a:x:b: OPTION "$@"; do
    case $OPTION in
    d)
        set -x
        ;;
    i)
        DMGASSETSPATH="${OPTARG}"
        ;;
    e)
        ENTITLEMENTPLISTPATH="${OPTARG}"
        ;;
    a)
        ARM64BUNDLEPATH="${OPTARG}"
        ;;
    x)
        X8664BUNDLEPATH="${OPTARG}"
        ;;
    b)
        BUILDPATH="${OPTARG}"
        ;;
    *)
        ;;
    esac
done

if [ -z "$ENTITLEMENTPLISTPATH" ]; then
    echo "Could not find entitlements.plist"
    echo "Try pointing to the extras/macos/entitlements.plist in the naev source tree"
    exit 1
fi

# Creates temp dir if needed
if [ -z "$BUILDPATH" ]; then
    BUILDPATH="$(mktemp -d)"
    WORKPATH=$(readlink -mf "$BUILDPATH")
else
    WORKPATH=$(readlink -mf "$BUILDPATH")
fi

BUILDPATH="$WORKPATH/builddir"

# Output configured variables

echo "SCRIPT WORKING PATH:  $WORKPATH"
echo "ARM64BUNDLEPATH PATH: $ARM64BUNDLEPATH"
echo "X8664BUNDLEPATH PATH: $X8664BUNDLEPATH"
echo "BUILD PATH:           $BUILDPATH"

# Make temp directories
mkdir -p "$WORKPATH"/dist
mkdir -p "$BUILDPATH"

check_tools(){
    # Checks if we have everything we need, which in this case is the lipo tool and the rcodesign tool available in PATH.
    if ! [ -x "$(command -v lipo)" ]; then
        echo "You don't have lipo in PATH"
        echo "Try running on a new enough version of macOS or make sure you have OSXCross installed."
        exit 1
    elif ! [ -x "$(command -v rcodesign)" ]; then
        echo "You don't have rcodesign in PATH"
        echo "Get it from https://github.com/indygreg/apple-platform-rs/releases"
        exit 1
    fi
}

fatten_libraries(){
    # Create working directories
    mkdir -p "$BUILDPATH"/{Frameworks.arm64,Frameworks.x86_64,Frameworks.Universal}

    # Collect and thin out other unneeded architectures from dylibs (if needed) for each architecture.
    for f in "$X8664BUNDLEPATH"/Naev.app/Contents/Frameworks/*
    do
        if [[ $(lipo "$f" -archs ) == "x86_64" ]]; then
            echo "$(basename "$f") already contains only the x86_64 architecture."
            cp "$f" "$BUILDPATH/Frameworks.x86_64"
        else
            echo "Extracting x86_64 arch from $(basename "$f")"
            lipo "$f" -extract x86_64 -output "$BUILDPATH"/Frameworks.x86_64/"$(basename "$f")"
        fi
    done
    for f in "$ARM64BUNDLEPATH"/Naev.app/Contents/Frameworks/*
    do
        if [[ $(lipo "$f" -archs) == "arm64" ]]; then
            echo "$(basename "$f") already contains only the arm64 architecture."
            cp "$f" "$BUILDPATH/Frameworks.arm64"
        else
            echo "Extracting arm64 arch from $(basename "$f")"
            lipo "$f" -extract arm64 -output "$BUILDPATH"/Frameworks.arm64/"$(basename "$f")"
        fi
    done

    # Combine both arches into a universal binary (if that library exists for both) otherwise copy ARM64 library
    # This should work well since ARM64 appears to depend on more libraries than x86_64, but also requires the same libraries.
    # TODO, make this more robust, if possible (Probably is a way)

    for f in "$BUILDPATH"/Frameworks.arm64/*
    do
        if [[ -f "$BUILDPATH/Frameworks.x86_64/$(basename "$f")" ]]; then
            echo "Combining ARM64 and x86_64 versions of $(basename "$f")"
            lipo "$f" "$BUILDPATH/Frameworks.x86_64/$(basename "$f")" -create -output "$BUILDPATH"/Frameworks.Universal/"$(basename "$f")"
        else
            echo "Copying $(basename "$f") as it is exclusively for ARM64"
            cp "$f" "$BUILDPATH"/Frameworks.Universal
        fi
    done
}

fatten_binaries(){
    # Create working directories
    mkdir -p "$BUILDPATH"/{naev.arm64,naev.x86_64,naev.Universal}

    # Copy x86_64 and ARM64 binaries to respective working directories
    cp "$X8664BUNDLEPATH"/Naev.app/Contents/MacOS/naev "$BUILDPATH"/naev.x86_64
    cp "$ARM64BUNDLEPATH"/Naev.app/Contents/MacOS/naev "$BUILDPATH"/naev.arm64

    # Combine both binaries into Universal binary
    echo "Combining ARM64 and x86_64 naev binaries"
    lipo "$BUILDPATH/naev.x86_64/naev" "$BUILDPATH/naev.arm64/naev" -create -output "$BUILDPATH/naev.Universal/naev"
}

sign_bundle(){
    rcodesign sign -v -e "$ENTITLEMENTPLISTPATH" "$BUILDPATH/Naev.app"
}

build_bundle(){
    # Create placeholder app bundle
    cp -r "$ARM64BUNDLEPATH/Naev.app" "$BUILDPATH/Naev.app"
    rm "$BUILDPATH"/Naev.app/Contents/MacOS/naev
    rm "$BUILDPATH"/Naev.app/Contents/Frameworks/*

    # Deploy Universal and extra ARM64 libraries
    fatten_libraries
    cp "$BUILDPATH"/Frameworks.Universal/* "$BUILDPATH/Naev.app/Contents/Frameworks"

    # Deploy Universal Naev binary.
    fatten_binaries
    cp "$BUILDPATH/naev.Universal/naev" "$BUILDPATH/Naev.app/Contents/MacOS"

    # Sign Universal bundle
    sign_bundle
}

zip_bundle(){
    pushd "$BUILDPATH/Naev.app"
    cd ../
    zip -r "$WORKPATH"/dist/naev-macos.zip Naev.app/*
    popd
}

build_dmg(){
    # Create working directories
    mkdir -p "$BUILDPATH/dmg_staging"

    # Create Applications symlink for DMG installer in dmg staging area
    ln -s /Applications "$BUILDPATH"/dmg_staging/Applications

    # Copy all DMG installer assets to dmg staging area
    cp -r "$DMGASSETSPATH"/. "$BUILDPATH/dmg_staging"

    # Copy Universal Naev app bundle to dmg staging area
    cp -r "$BUILDPATH/Naev.app" "$BUILDPATH/dmg_staging"

    # Generate ISO image and compress into DMG
    genisoimage -V Naev -D -R -apple -no-pad -o "$BUILDPATH"/naev-macos.iso "$BUILDPATH/dmg_staging"
    dmg "$BUILDPATH"/naev-macos.iso "$WORKPATH"/dist/naev-macos.dmg
}

package_bundle(){
    # Zip Naev.app for distribution
    zip_bundle
    # Generate Naev DMG installer for distribution
    build_dmg
}

check_tools
build_bundle
package_bundle
