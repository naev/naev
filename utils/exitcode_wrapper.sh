#!/bin/bash
"$@"
[ $? -ge 1 ] && exit 0 || exit 1
