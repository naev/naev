#!/bin/bash
rm ndata
./autogen.sh && ./configure
make
sudo make install
