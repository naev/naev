#!/usr/bin/env bash

# Upload a release and assets to a Forgejo (Gitea-compatible) instance.
# Defaults target Codeberg, but everything can be overridden for other servers.

set -euo pipefail

usage() {
   cat <<'EOF'
Usage: upload_forgejo_release.sh --tag TAG --asset-dir DIR [options]

Required:
  --tag TAG            Release tag to create/update.
  --asset-dir DIR      Directory containing assets to upload.

Optional:
  --title TITLE        Release title (defaults to tag).
  --notes TEXT         Release notes (inline text).
  --notes-file FILE    Release notes file (overrides --notes).
  --owner OWNER        Repository owner (default: $FORGEJO_OWNER or naev).
  --repo REPO          Repository name (default: $FORGEJO_REPO or naev).
  --api-url URL        API base (default: $FORGEJO_API_URL or https://codeberg.org/api/v1).
  --prerelease         Mark release as prerelease.
  --overwrite          Delete existing release with same tag first.
  --hide-archive-link  Hide auto-generated source archives on the release.

Auth:
   FORGEJO_TOKEN (preferred), else CODEBERG_TOKEN, else GITHUB_TOKEN.
EOF
   exit 1
}

TAG=""
TITLE=""
BODY=""
BODY_FILE=""
ASSET_DIR=""
OWNER="${FORGEJO_OWNER:-naev}"
REPO="${FORGEJO_REPO:-naev}"
API_URL="${FORGEJO_API_URL:-https://codeberg.org/api/v1}"
PRERELEASE="false"
OVERWRITE="false"
HIDE_ARCHIVE="false"

while [[ $# -gt 0 ]]; do
   case "$1" in
      --tag) TAG="$2"; shift 2 ;;
      --title) TITLE="$2"; shift 2 ;;
      --notes) BODY="$2"; shift 2 ;;
      --notes-file) BODY_FILE="$2"; shift 2 ;;
      --asset-dir) ASSET_DIR="$2"; shift 2 ;;
      --owner) OWNER="$2"; shift 2 ;;
      --repo) REPO="$2"; shift 2 ;;
      --api-url) API_URL="$2"; shift 2 ;;
      --prerelease) PRERELEASE="true"; shift 1 ;;
      --overwrite) OVERWRITE="true"; shift 1 ;;
      --hide-archive-link) HIDE_ARCHIVE="true"; shift 1 ;;
      -h|--help) usage ;;
      *) usage ;;
   esac
done

TOKEN="${FORGEJO_TOKEN:-${CODEBERG_TOKEN:-${GITHUB_TOKEN:-}}}"
if [[ -z "$TOKEN" ]]; then
   echo "error: FORGEJO_TOKEN (or CODEBERG_TOKEN / GITHUB_TOKEN) must be set" >&2
   exit 1
fi

if [[ -z "$TAG" ]] || [[ -z "$ASSET_DIR" ]]; then
   usage
fi

if [[ -n "$BODY_FILE" ]]; then
   BODY="$(<"$BODY_FILE")"
fi

if [[ -z "$TITLE" ]]; then
   TITLE="$TAG"
fi

if [[ ! -d "$ASSET_DIR" ]]; then
   echo "error: asset dir '$ASSET_DIR' not found" >&2
   exit 1
fi

auth_header=(-H "Authorization: token ${TOKEN}")
json_header=(-H "Content-Type: application/json")

retry() {
   local attempts="$1"; shift
   local delay=1
   local i=1
   while (( i <= attempts )); do
      if "$@"; then
         return 0
      fi
      echo "warn: attempt $i/$attempts failed, retrying in ${delay}s..." >&2
      sleep "$delay"
      delay=$((delay*2))
      i=$((i+1))
   done
   return 1
}

api() {
   local method="$1"; shift
   curl -fsSL -X "$method" "${auth_header[@]}" "$@"
}

api_retry() {
   local method="$1"; shift
   retry 5 api "$method" "$@"
}

# Optionally delete existing release
if [[ "$OVERWRITE" == "true" ]]; then
   tmp_resp="$(mktemp)"
   if curl -s -w "%{http_code}" -o "$tmp_resp" "${auth_header[@]}" \
         "$API_URL/repos/$OWNER/$REPO/releases/tags/$TAG" | grep -q "^200$"; then
      existing_id="$(jq -r '.id // empty' "$tmp_resp")"
      if [[ -n "$existing_id" ]]; then
         api_retry DELETE "$API_URL/repos/$OWNER/$REPO/releases/$existing_id" >/dev/null
      fi
   fi
   rm -f "$tmp_resp"
fi

# Create release
payload="$(jq -n \
   --arg tag "$TAG" \
   --arg name "$TITLE" \
   --arg body "$BODY" \
   --argjson prerelease "$PRERELEASE" \
   --argjson hide "$HIDE_ARCHIVE" \
   '{tag_name:$tag, name:$name, body:$body, draft:false, prerelease:$prerelease, hide_archive_links:$hide}')"

release_resp="$(api_retry POST "$API_URL/repos/$OWNER/$REPO/releases" "${json_header[@]}" -d "$payload")"
release_id="$(echo "$release_resp" | jq -r '.id // empty')"

if [[ -z "$release_id" ]]; then
   echo "error: failed to create release: $release_resp" >&2
   exit 1
fi

echo "Created release id $release_id for tag $TAG on $API_URL"

# Upload assets
shopt -s nullglob
for file in "$ASSET_DIR"/*; do
   if [[ -f "$file" ]]; then
      name="$(basename "$file")"
      echo "Uploading asset: $name"
      api_retry POST "$API_URL/repos/$OWNER/$REPO/releases/$release_id/assets?name=$name" \
         -H "Content-Type: application/octet-stream" \
         --data-binary @"$file" >/dev/null
   fi
done

echo "Release upload complete."
