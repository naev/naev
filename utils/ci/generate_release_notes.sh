#!/usr/bin/env bash

# Generate release notes from a changelog section or from a git log range.
# Outputs a markdown file; intended for use in CI workflows.

set -euo pipefail

usage() {
   cat <<'EOF'
Usage:
  generate_release_notes.sh --mode changelog --tag TAG --changelog FILE --output FILE
  generate_release_notes.sh --mode nightly --repo-path PATH --prev-sha SHA --build-sha SHA --build-date DATE --output FILE [--repo-url URL]

Options:
  --mode changelog           Extract section matching "## TAG" from changelog.
  --mode nightly             Build nightly notes from git log range prev..build.
  --tag TAG                  Version/tag string to match in changelog.
  --changelog FILE           Path to Changelog.md.
  --repo-path PATH           Path to git repository (for nightly mode).
  --prev-sha SHA             Previous SHA (exclusive) for git log range.
  --build-sha SHA            Current build SHA.
  --build-date DATE          Current build date (e.g., 2024-01-01).
  --repo-url URL             Base commit URL (default: https://codeberg.org/naev/naev/commit).
  --output FILE              Where to write the notes.
EOF
   exit 1
}

MODE=""
TAG=""
CHANGELOG=""
OUTPUT=""
REPO_PATH=""
PREV_SHA=""
BUILD_SHA=""
BUILD_DATE=""
REPO_URL="https://codeberg.org/naev/naev/commit"

while [[ $# -gt 0 ]]; do
   case "$1" in
      --mode) MODE="$2"; shift 2 ;;
      --tag) TAG="$2"; shift 2 ;;
      --changelog) CHANGELOG="$2"; shift 2 ;;
      --output) OUTPUT="$2"; shift 2 ;;
      --repo-path) REPO_PATH="$2"; shift 2 ;;
      --prev-sha) PREV_SHA="$2"; shift 2 ;;
      --build-sha) BUILD_SHA="$2"; shift 2 ;;
      --build-date) BUILD_DATE="$2"; shift 2 ;;
      --repo-url) REPO_URL="$2"; shift 2 ;;
      -h|--help) usage ;;
      *) echo "error: unknown arg $1" >&2; usage ;;
   esac
done

if [[ -z "$MODE" ]] || [[ -z "$OUTPUT" ]]; then
   usage
fi

mkdir -p "$(dirname "$OUTPUT")"

case "$MODE" in
   changelog)
      if [[ -z "$TAG" ]] || [[ -z "$CHANGELOG" ]]; then usage; fi
      if [[ ! -f "$CHANGELOG" ]]; then
         echo "error: changelog '$CHANGELOG' not found" >&2
         exit 1
      fi
      notes="$(awk -v tag="## ${TAG}" '
         BEGIN {found=0}
         /^## / {
           if (found) exit
           if ($0 == tag) {found=1; next}
         }
         found {print}
      ' "$CHANGELOG")"
      if [[ -z "$notes" ]]; then
         echo "warn: changelog section for ${TAG} not found, using full changelog" >&2
         notes="$(cat "$CHANGELOG")"
      fi
      {
         echo "# ${TAG}"
         echo
         echo "$notes"
      } > "$OUTPUT"
      ;;
   nightly)
      if [[ -z "$REPO_PATH" ]] || [[ -z "$PREV_SHA" ]] || [[ -z "$BUILD_SHA" ]] || [[ -z "$BUILD_DATE" ]]; then
         usage
      fi
      if [[ ! -d "$REPO_PATH/.git" ]]; then
         echo "error: repo path '$REPO_PATH' is not a git repository" >&2
         exit 1
      fi
      pushd "$REPO_PATH" >/dev/null
      log_entries=$(git log --pretty=format:"- %s ([%h](${REPO_URL}/%H))" "${PREV_SHA}..${BUILD_SHA}")
      popd >/dev/null
      {
         echo "## Git SHA: [${BUILD_SHA:0:7}](${REPO_URL}/${BUILD_SHA}) Built on ${BUILD_DATE}"
         echo
         echo "This is an automated development snapshot build. Debugging is enabled."
         echo
         echo "Please report any issues you run into on the [issue tracker](https://codeberg.org/naev/naev/issues)!"
         echo
         echo "### Changes since last build"
         echo
         echo "$log_entries"
      } > "$OUTPUT"
      ;;
   *)
      usage
      ;;
esac

echo "Wrote release notes to $OUTPUT"
