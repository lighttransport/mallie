#!/bin/sh

flags="--k --with-openmp"
./tools/solaris/premake4 ${flags} gmake

# change CC, CXX compiler to fccpx, FCCpx
sed -i -e 's/ifndef CC/CC=fcc\nifndef CC/g' Mallie.make
sed -i -e 's/ifndef CXX/CXX=FCC\nifndef CXX/g' Mallie.make
