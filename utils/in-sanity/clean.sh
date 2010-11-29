#!/usr/bin/env bash

echo "Remove backup file:"
find . -type f -name "*~" -exec rm -f {} \;
