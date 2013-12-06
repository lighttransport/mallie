#!/bin/bash

flags='--arm'

./tools/linux/premake4 ${flags} gmake

# change CC, CXX compiler
sed -i -e 's/ifndef CC/CC=aarch64-linux-gnu-gcc\nifndef CC/g' Mallie.make
sed -i -e 's/ifndef CXX/CXX=aarch64-linux-gnu-g++\nifndef CXX/g' Mallie.make
