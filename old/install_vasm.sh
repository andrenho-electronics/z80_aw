#!/bin/sh

pushd .
wget http://sun.hasenbraten.de/vasm/release/vasm.tar.gz
tar zxpvf vasm.tar.gz
cd vasm
make CPU=z80 SYNTAX=oldstyle
sudo install vasmz80_oldstyle /usr/local/bin
cd ..
rm -rf vasm
popd
