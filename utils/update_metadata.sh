#!/usr/bin/env bash
#
# update_metadata.sh  Update Naev AppStream metadata and regenerate desktop file
#
# Usage (Run from source root):
#   ./utils/update_metadata.sh [--devel] <version> <release_url>
#
# Examples:
#   ./utils/update_metadata.sh 0.12.6 https://naev.org/blarg/2025-07-09_0.12.6/
#   ./utils/update_metadata.sh --devel 0.13.0-beta1 https://naev.org/blarg/2026-01-02_0.13.0-beta1/
#
# Requires: appstreamcli, grep, xmlstarlet

set -euo pipefail

# Ensure tools read/write UTF-8 (prevents &#xNNNN; escapes)
export LC_ALL=C.UTF-8
export LANG=C.UTF-8

METAINFO="org.naev.Naev.metainfo.xml"
DESKTOP="org.naev.Naev.desktop"
#APP_ID="org.naev.Naev"

TYPE="stable"
ARGS=()

for arg in "$@"; do
   case "$arg" in
      --devel|--dev|--development)
         TYPE="development"
         ;;
      *)
         ARGS+=("$arg")
         ;;
   esac
done

if [ "${#ARGS[@]}" -ne 2 ]; then
   echo "Usage: $0 [--devel] <version> <release_notes_url>" >&2
   exit 1
fi

VERSION="${ARGS[0]}"
URL="${ARGS[1]}"

# Extract the ISO date (YYYY-MM-DD) from the URL, e.g. 2025-07-09_0.12.6
if [[ "$URL" =~ ([0-9]{4}-[0-9]{2}-[0-9]{2})_ ]]; then
   DATE="${BASH_REMATCH[1]}"
else
   echo "Warning: could not extract release date from URL, using current date" >&2
   DATE="$(date +%Y-%m-%d)"
fi

echo "Version: $VERSION"
echo "Type: $TYPE"
echo "URL: $URL"
echo "Date: $DATE"

if [ ! -f "$METAINFO" ]; then
   echo "Error: $METAINFO not found" >&2
   exit 1
fi

for cmd in appstreamcli xmlstarlet; do
   if ! command -v "$cmd" >/dev/null; then
      echo "Error: required tool '$cmd' not found in PATH" >&2
      exit 1
   fi
done

# Ensure a <releases> element exists
if ! xmlstarlet sel -t -v "count(/component/releases)" "$METAINFO" | grep -q '^1$'; then
   echo "Creating <releases> section"
   xmlstarlet ed -L -P \
      -s "/component" -t elem -n "releases" -v "" \
      "$METAINFO"
fi

# Remove any existing release with same version first
xmlstarlet ed -L -P \
   -d "/component/releases//release[@version='$VERSION']" \
   "$METAINFO"

echo "Inserting new <release version=\"$VERSION\"> entry"

# Check how many child <release> nodes exist
RELCOUNT="$(xmlstarlet sel -t -v 'count(/component/releases/release)' "$METAINFO")"

if [ "${RELCOUNT:-0}" -ge 1 ]; then
   # Insert a new sibling BEFORE the current first <release>
   xmlstarlet ed -L -P \
      -i "/component/releases/release[1]" -t elem -n "release" -v "" \
      -i "/component/releases/release[1]" -t attr -n "version" -v "$VERSION" \
      -i "/component/releases/release[1]" -t attr -n "type"    -v "$TYPE" \
      -i "/component/releases/release[1]" -t attr -n "date"    -v "$DATE" \
      -s "/component/releases/release[1]" -t elem -n "url" -v "$URL" \
      -i "/component/releases/release[1]/url" -t attr -n "type" -v "details" \
      "$METAINFO"
else
   # No releases yet: create the first one as a child of <releases>
   xmlstarlet ed -L -P \
      -s "/component/releases" -t elem -n "release" -v "" \
      -i "/component/releases/release[1]" -t attr -n "version" -v "$VERSION" \
      -i "/component/releases/release[1]" -t attr -n "type"    -v "$TYPE" \
      -i "/component/releases/release[1]" -t attr -n "date"    -v "$DATE" \
      -s "/component/releases/release[1]" -t elem -n "url" -v "$URL" \
      -i "/component/releases/release[1]/url" -t attr -n "type" -v "details" \
      "$METAINFO"
fi

# Reformat XML (We use 2 space tabs)
xmlstarlet fo -s 2 "$METAINFO" > "${METAINFO}.tmp" && mv "${METAINFO}.tmp" "$METAINFO"

echo "Regenerating desktop file"
appstreamcli make-desktop-file "$METAINFO" "$DESKTOP"

echo "Validating metainfo"
if ! appstreamcli validate --pedantic "$METAINFO"; then
   echo "Validation warnings detected: please review above output"
fi

echo "Updated $METAINFO and $DESKTOP for $VERSION ($DATE, $TYPE)"
