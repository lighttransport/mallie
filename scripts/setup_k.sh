#!/bin/sh

flags="--k --with-openmp"
./tools/linux/premake4 ${flags} gmake

# change CC, CXX compiler to fccpx, FCCpx
sed -i -e 's/ifndef CC/CC=fccpx\nifndef CC/g' Mallie.make
sed -i -e 's/ifndef CXX/CXX=FCCpx\nifndef CXX/g' Mallie.make
