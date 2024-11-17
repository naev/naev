#!/bin/bash

# After building Naev.app, use this script to sign the bundle. Requires a
# single argument: the signing identity. This should be a code-signing
# certificate present in the default keychain.

# Official Naev builds are currently not yet signed, so this script is right
# now just an example.

exec rcodesign sign -v -e extras/macos/entitlements.plist "$1" \
  Naev.app

# Old invokation that works with native Apple signing tool

#codesign --verbose --force --deep --sign "$1" \
#  --entitlements extras/macos/entitlements.plist \
#  Naev.app
