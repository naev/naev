#!/usr/bin/env bash

set -e

usage() {
   cat <<EOF
Usage: $(basename "$0") [options]

Manually sends a workflow dispatch to Forgejo.

Required:
  -r <release_type>  Release type: nightly | prerelease | release
  -t <token>         Forgejo application token (requires repo read+write scope).
                     Can also be set via environment variable FJ_TOKEN.

Optional:
  -g <repo>          Repository name (default: naev/naev)
  -b <ref>           Branch or ref name (default: main)
  -s <server_url>    Forgejo instance URL (default: https://codeberg.org or \$SERVER_URL)
  -d                 Enable debug mode (set -x)

Examples:
   $(basename "$0") -t <token> -r nightly
   FJ_TOKEN=<token> $(basename "$0") -r release -s https://git.myforgejo.org

EOF
   exit 1
}

# Defaults
REPO="naev/naev"
REF="main"
FORGEJO_SERVER_URL="${FORGEJO_SERVER_URL:-https://codeberg.org}"
FORGEJO_API_URL="${FORGEJO_SERVER_URL%/}/api/v1"

while getopts dt:r:g:b:s: OPTION; do
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
   s)
      FORGEJO_SERVER_URL="${OPTARG}"
      ;;
   *)
      usage
      ;;
   esac
done

# If TOKEN was not provided with -t, fall back to FJ_TOKEN
if [[ -z "$TOKEN" && -n "$FJ_TOKEN" ]]; then
   TOKEN="$FJ_TOKEN"
fi

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
   RESPONSE=$(curl --fail -s -X POST "${FORGEJO_API_URL}/repos/${REPO}/actions/workflows/${WORKFLOW_FILE_NAME}/dispatches" \
      -H "Accept: application/json" \
      -H "Content-Type: application/json" \
      -H "Authorization: token ${TOKEN}" \
      --data "{\"ref\":\"${REF}\", \"return_run_info\": true}")

   RUN_NUMBER=$(echo "$RESPONSE" | grep -o '"run_number"[ ]*:[ ]*[0-9]*' | grep -o '[0-9]*')
   if [[ -n "$RUN_NUMBER" ]]; then
      echo "View run progress at: ${FORGEJO_SERVER_URL}/${REPO}/actions/runs/${RUN_NUMBER}"
   else
      echo "Workflow dispatched, but could not determine run number."
   fi
}

trigger_workflow
