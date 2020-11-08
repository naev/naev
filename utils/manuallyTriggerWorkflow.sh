#!/bin/bash
# Manually runs the nightly workflow when sent.

# Pass in -t <personalAPItoken>

set -e

while getopts d:t: OPTION "$@"; do
    case $OPTION in
    d)
        set -x
        ;;
    t)
        TOKEN="${OPTARG}"
        ;;
    esac
done

if [[ -z "$TOKEN" ]]; then
    echo "usage: `basename $0` [-d] -t <personalAPItoken>"
    exit 1
fi

curl \
  -X POST \
  -H "Accept: application/vnd.github.v3+json" \
  -H "Authorization: token $TOKEN" \
  https://api.github.com/repos/naev/naev/dispatches \
  -d '{"event_type":"manual-nightly"}'
