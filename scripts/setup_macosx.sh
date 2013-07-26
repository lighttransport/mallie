#!/bin/bash

flags='--with-sdl --with-openmp'

./tools/macosx/premake4 ${flags} gmake

# change CC, CXX compiler
gsed -i -e 's/ifndef CC/CC=gcc-4.8\nifndef CC/g' Mallie.make
gsed -i -e 's/ifndef CXX/CXX=g++-4.8\nifndef CXX/g' Mallie.make
