#!/bin/bash

args=`echo "$@" | tr -d "\r\n"`

if [ -n "${args}" ]; then
	flags="${args}"
else
	flags='--with-sdl --with-openmp'
fi


echo premake flags: ${flags}

./tools/macosx/premake4 ${flags} gmake

# change CC, CXX compiler
gsed -i -e 's/ifndef CC/CC=gcc-4.8\nifndef CC/g' Mallie.make
gsed -i -e 's/ifndef CXX/CXX=g++-4.8\nifndef CXX/g' Mallie.make
