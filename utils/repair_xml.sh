#!/bin/bash

# Repair the side - effects of using elementTree
sed -i -e 's/\(<[^>]*\) \/>/\1\/>/' -e '$a'"\\" "$@"
