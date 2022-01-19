#!/bin/bash

set -e

usage() {
    echo "usage: $(basename "$0") [-d] -t <personalAPItoken> -r <releasetype, (nightly, prerelease, release)>"
    echo "Manually runs the nightly workflow when sent."
    echo "Pass in -t <personalAPItoken> -r <releasetype, (nightly, prerelease, release)> -g <github repo name e.g. (naev/naev)>"
    exit 1
}

# Defaults
REPO="naev/naev"

while getopts d:t:r:g: OPTION "$@"; do
    case $OPTION in
    d)
        set -x
        ;;
    t)
        TOKEN="${OPTARG}"
        ;;
    r)
        RELEASETYPE="${OPTARG}"
        ;;
    g)
        REPO="${OPTARG}"
        ;;
    *)
        usage
        ;;
    esac
done

if [[ -z "$TOKEN" ]] || [[ -z "$RELEASETYPE" ]]; then
    usage
fi

if [[ "$RELEASETYPE" == "nightly" ]]; then
  curl \
    -X POST \
    -H "Accept: application/vnd.github.v3+json" \
    -H "Authorization: token $TOKEN" \
    https://api.github.com/repos/"$REPO"/dispatches \
    -d '{"event_type":"manual-nightly"}'
elif [[ "$RELEASETYPE" == "prerelease" ]]; then
  curl \
    -X POST \
    -H "Accept: application/vnd.github.v3+json" \
    -H "Authorization: token $TOKEN" \
    https://api.github.com/repos/"$REPO"/dispatches \
    -d '{"event_type":"manual-prerelease"}'
elif [[ "$RELEASETYPE" == "release" ]]; then
  curl \
    -X POST \
    -H "Accept: application/vnd.github.v3+json" \
    -H "Authorization: token $TOKEN" \
    https://api.github.com/repos/"$REPO"/dispatches \
    -d '{"event_type":"manual-release"}'
else
    echo "-r must be either nightly, prerelease or release"
    exit 1
fi
