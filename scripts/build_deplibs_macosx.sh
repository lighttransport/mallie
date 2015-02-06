#!/bin/sh

curdir=`pwd`
targetdir="${curdir}"/extlibs/macosx/SDL2

cd deps/SDL2-2.0.3
rm -rf mybuild
mkdir mybuild
cd mybuild
../configure --prefix="${targetdir}" --disable-shared
make && make install

cd "${curdir}"

