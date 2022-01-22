#!/bin/bash

set -e

usage() {
    echo "usage: $(basename "$0") [-d] -t <personalAPItoken> -r <releasetype, (nightly, prerelease, release)>"
    echo "Manually sends a workflow dispatch when run."
    echo "PAT Requires 'workflow' scope"
    echo "Pass in -t <Github PAT> -r <release type> e.g. (nightly, prerelease, release) -g <github repo name> defaults to: (naev/naev) -b <ref/branch name> defaults to: (main)"
    exit 1
}

# Defaults
REPO="naev/naev"
REF="main"
GITHUB_API_URL="${API_URL:-https://api.github.com}"
GITHUB_SERVER_URL="${SERVER_URL:-https://github.com}"

while getopts dt:r:g:b: OPTION "$@"; do
    case $OPTION in
    d)
        set -x
        ;;
    t)
        TOKEN="${OPTARG}"
        ;;
    r)
        RELEASE_TYPE="${OPTARG}"
        ;;
    g)
        REPO="${OPTARG}"
        ;;
    b)
        REF="${OPTARG}"
        ;;
    *)
        usage
        ;;
    esac
done

if [[ -z "$TOKEN" ]] || [[ -z "$RELEASE_TYPE" ]]; then
    usage
fi

case $RELEASE_TYPE in
  "nightly")
    WORKFLOW_FILE_NAME="naev_$RELEASE_TYPE.yml"
    ;;
  "prerelease")
    WORKFLOW_FILE_NAME="naev_$RELEASE_TYPE.yml"
    ;;
  "release")
    WORKFLOW_FILE_NAME="naev_$RELEASE_TYPE.yml"
    ;;
  *)
    echo "-r must be either nightly, prerelease or release"
    exit 1
    ;;
esac

trigger_workflow() {
  echo "View run progress at: ${GITHUB_SERVER_URL}/${REPO}/actions/workflows/${WORKFLOW_FILE_NAME}"

  curl --fail -X POST "${GITHUB_API_URL}/repos/${REPO}/actions/workflows/${WORKFLOW_FILE_NAME}/dispatches" \
    -H "Accept: application/vnd.github.v3+json" \
    -H "Content-Type: application/json" \
    -H "Authorization: Bearer ${TOKEN}" \
    --data "{\"ref\":\"${REF}\"}"
}

trigger_workflow
