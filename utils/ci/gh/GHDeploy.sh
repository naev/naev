#!/usr/bin/env bash

# GITHUB DEPLOYMENT SCRIPT FOR NAEV
#
# This script should be run after downloading all build artefacts.
# It also makes use of ENV variable GH_TOKEN to login. ensure this is exported.
#
#
# Pass in [-d] [-n] (set this for nightly builds) [-p] (set this for pre-release builds.) [-c] (set this for CI testing) -t <TEMPPATH> (build artefact location) -o <OUTDIR> (dist output directory) -r <TAGNAME> (tag of release *required*) -g <REPONAME> (defaults to naev/naev)

set -e

# Defaults
NIGHTLY="false"
PRERELEASE="false"
TEMPPATH="$(pwd)"
OUTDIR="$(pwd)/dist"
DRYRUN="false"
REPONAME="naev/naev"

while getopts dnpct:o:r:g: OPTION "$@"; do
    case $OPTION in
    d)
        set -x
        ;;
    n)
        NIGHTLY="true"
        ;;
    p)
        PRERELEASE="true"
        ;;
    c)
        DRYRUN="true"
        ;;
    t)
        TEMPPATH="${OPTARG}"
        ;;
    o)
        OUTDIR="${OPTARG}"
        ;;
    r)
        TAGNAME="${OPTARG}"
        ;;
    g)
        REPONAME="${OPTARG}"
        ;;
    *)
        ;;
    esac
done

if [[ -z "$TAGNAME" ]]; then
    echo "usage: $(basename "$0") [-d] [-n] (set this for nightly builds) [-p] (set this for pre-release builds.) [-c] (set this for CI testing) -t <TEMPPATH> (build artefact location) -o <OUTDIR> (dist output directory) -r <TAGNAME> (tag of release *required*) -g <REPONAME> (defaults to naev/naev)"
    exit 1
fi

# Check dependencies
if ! [ -x "$(command -v gh)" ]; then
    echo "You don't have gh (GitHub CLI) in PATH"
    exit 1
fi
if ! [ -x "$(command -v git)" ]; then
    echo "You don't have git in PATH"
    exit 1
fi

# Set git configuration from environment variables if provided
if [ -n "$GIT_USERNAME" ] && [ -n "$GIT_EMAIL" ]; then
    git -C build/staging/repo config user.name "$GIT_USERNAME"
    git -C build/staging/repo config user.email "$GIT_EMAIL"
fi

VERSION="$(<"$TEMPPATH/naev-version/VERSION")"

# Make dist path if it does not exist
mkdir -p "$OUTDIR"/dist
mkdir -p "$OUTDIR"/lin64
mkdir -p "$OUTDIR"/macos
mkdir -p "$OUTDIR"/win64
mkdir -p "$OUTDIR"/soundtrack

# Move all build artefacts to deployment locations
# Move Linux AppImage, zsync files and set AppImage as executable
cp "$TEMPPATH"/naev-linux-x86-64/*.AppImage "$OUTDIR"/lin64/naev-"$VERSION"-linux-x86-64.AppImage
cp "$TEMPPATH"/naev-linux-x86-64/*.zsync "$OUTDIR"/lin64/naev-"$VERSION"-linux-x86-64.AppImage.zsync

chmod +x "$OUTDIR"/lin64/naev-"$VERSION"-linux-x86-64.AppImage

# Move macOS dmg image to deployment location
cp "$TEMPPATH"/naev-macos/*.dmg "$OUTDIR"/macos/naev-"$VERSION"-macos-universal.dmg

# Move Windows installer to deployment location
cp "$TEMPPATH"/naev-win64/naev*.exe "$OUTDIR"/win64/naev-"$VERSION"-win64.exe

# Move Dist to deployment location
cp "$TEMPPATH"/naev-dist/source.tar.xz "$OUTDIR"/dist/naev-"$VERSION"-source.tar.xz

# Move Soundtrack to deployment location if this is a release.
if [ "$NIGHTLY" == "true" ] || [ "$PRERELEASE" == "true" ]; then
    echo "not preparing soundtrack"
else
    cp "$TEMPPATH"/naev-soundtrack/naev-*-soundtrack.zip "$OUTDIR"/dist/naev-"$VERSION"-soundtrack.zip
fi

# Create Release and push builds to github via gh
# If DRYRUN is set to true, simulate the release creation and upload

if [ "$DRYRUN" == "false" ]; then
    # Delete existing release for the current tag if it exists
    if gh release view "$TAGNAME" --repo "$REPONAME" >/dev/null 2>&1; then
        gh release delete "$TAGNAME" --repo "$REPONAME" --yes
    fi

    # For nightly releases, force push the "nightly" tag to HEAD
    if [ "$NIGHTLY" == "true" ]; then
        pushd build/staging/repo > /dev/null
        git tag -f nightly HEAD
        git push -f origin nightly
        popd > /dev/null
    fi

    # Create release for $TAGNAME
    if [ "$NIGHTLY" == "true" ]; then
        gh release create "$TAGNAME" --title "Nightly Build" --prerelease --generate-notes --verify-tag --repo "$REPONAME"
    else
        if [ "$PRERELEASE" == "true" ]; then
            gh release create "$TAGNAME" --title "$TAGNAME" --notes-file "build/staging/naev-changelog/Changelog.md" --prerelease --verify-tag --repo "$REPONAME"
        else
            gh release create "$TAGNAME" --title "$TAGNAME" --notes-file "build/staging/naev-changelog/Changelog.md" --verify-tag --repo "$REPONAME"
        fi
    fi

    gh --version
    gh release upload "$TAGNAME" "$OUTDIR/lin64/naev-${VERSION}-linux-x86-64.AppImage" --repo "$REPONAME" --clobber
    gh release upload "$TAGNAME" "$OUTDIR/lin64/naev-${VERSION}-linux-x86-64.AppImage.zsync" --repo "$REPONAME" --clobber
    gh release upload "$TAGNAME" "$OUTDIR/macos/naev-${VERSION}-macos-universal.dmg" --repo "$REPONAME" --clobber
    gh release upload "$TAGNAME" "$OUTDIR/win64/naev-${VERSION}-win64.exe" --repo "$REPONAME" --clobber
    if [ "$NIGHTLY" == "false" ] && [ "$PRERELEASE" == "false" ]; then
        gh release upload "$TAGNAME" "$OUTDIR/dist/naev-${VERSION}-soundtrack.zip" --repo "$REPONAME" --clobber
    fi
    gh release upload "$TAGNAME" "$OUTDIR/dist/naev-${VERSION}-source.tar.xz" --repo "$REPONAME" --clobber
elif [ "$DRYRUN" == "true" ]; then
    gh --version
    # Simulate deletion of an existing release for the current tag
    echo "Would check for and delete existing release for tag: $TAGNAME"
    if [ "$NIGHTLY" == "true" ]; then
        echo "Would force push 'nightly' tag to HEAD: git tag -f nightly HEAD && git push -f origin nightly"
        echo "Would create release: gh release create $TAGNAME --title 'Nightly Build' --prerelease --generate-notes --verify-tag --repo $REPONAME"
    else
        if [ "$PRERELEASE" == "true" ]; then
            echo "Would create release: gh release create $TAGNAME --title $TAGNAME --notes-file build/staging/naev-changelog/Changelog.md --prerelease --verify-tag --repo $REPONAME"
        else
            echo "Would create release: gh release create $TAGNAME --title $TAGNAME --notes-file build/staging/naev-changelog/Changelog.md --verify-tag --repo $REPONAME"
        fi
    fi
    # Simulate asset uploads
    echo "Would upload asset: gh release upload $TAGNAME $OUTDIR/lin64/naev-${VERSION}-linux-x86-64.AppImage --repo $REPONAME --clobber"
    echo "Would upload asset: gh release upload $TAGNAME $OUTDIR/lin64/naev-${VERSION}-linux-x86-64.AppImage.zsync --repo $REPONAME --clobber"
    echo "Would upload asset: gh release upload $TAGNAME $OUTDIR/macos/naev-${VERSION}-macos-universal.dmg --repo $REPONAME --clobber"
    echo "Would upload asset: gh release upload $TAGNAME $OUTDIR/win64/naev-${VERSION}-win64.exe --repo $REPONAME --clobber"
    if [ "$NIGHTLY" == "false" ] && [ "$PRERELEASE" == "false" ]; then
        echo "Would upload asset: gh release upload $TAGNAME $OUTDIR/dist/naev-${VERSION}-soundtrack.zip --repo $REPONAME --clobber"
    fi
    echo "Would upload asset: gh release upload $TAGNAME $OUTDIR/dist/naev-${VERSION}-source.tar.xz --repo $REPONAME --clobber"
    ls -l -R "$OUTDIR"
else
    echo "Something went wrong determining which mode to run this script in."
    exit 1
fi
