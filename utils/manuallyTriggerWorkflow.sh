#!/bin/bash
# Manually runs the nightly workflow when sent.

# Pass in -t <personalAPItoken> -r <releasetype, (nightly, prerelease, release)>

set -e

REPO="naikari/naikari"

while getopts d:t:r: OPTION "$@"; do
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
    esac
done

if [[ -z "$TOKEN" ]]; then
    echo "usage: `basename $0` [-d] -t <personalAPItoken> -r <releasetype, (nightly, prerelease, release)>"
    exit 1
elif [[ -z "$RELEASETYPE" ]]; then
    echo "usage: `basename $0` [-d] -t <personalAPItoken> -r <releasetype, (nightly, prerelease, release)>"
    exit 1
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

