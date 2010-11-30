#!/usr/bin/env bash

echo "Remove backup file:"
find . -type f -name "*~" -exec rm -f {} \;
find . -type f -name "*.py[oc]" -exec rm -f {} \;
