#!/bin/bash

flags='--with-sdl --with-openmp'

./tools/linux/premake4 ${flags} gmake
