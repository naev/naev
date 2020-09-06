#!/bin/bash
# Manually runs the nightly workflow when sent.

# Pass in -t <token>

set -e

while getopts d:w: OPTION "$@"; do
    case $OPTION in
    d)
        set -x
        ;;
    t)
        TOKEN=${OPTARG}
        ;;
    esac
done

curl -H "Accept: application/vnd.github.everest-preview+json" \
   -H "Authorization: token $TOKEN" \
   --request POST \
   --data '{"event_type": "manual-nightly"}' \
   https://api.github.com/repos/projectsynchro/naev/dispatches
