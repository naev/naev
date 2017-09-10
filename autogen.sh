#!/bin/sh
function autogen_error () {
   echo "Generating configuration files failed!"
   echo "Make sure you have the following packages installed:"
   echo ""
   echo "   automake"
   echo "   autopoint"
   echo "   inttool"
   exit 1
}
autoreconf -vif || autogen_error
intltoolize --force || autogen_error
