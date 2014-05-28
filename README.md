# Mallie ray tracer

## Building

### Requirements

* GCC (If you want OpenMP support)
* SDL2.0(optional)
* premake4
* ptex(optional)

#### Setup

Build ptex(optional)

    $ cd deps/ptex-master
    $ make

#### MacOSX

Edit scripts/setup_macosx.sh, then,

    $ ./scripts/setup_macosx.sh
    $ export CC=gcc48 # optional
    $ export CXX=g++48 # optional
    $ make
 
#### Linux

Edit scripts/setup_linux.sh, then,

    $ ./scripts/setup_linux.sh
    $ export CC=gcc48 # optional
    $ export CXX=g++48 # optional
    $ make

#### Windows

Edit vcbuild.bat, then,

    > vcbuid.bat

Solition file will be generated.
Tested on VS2013.

#### Bootstrap options for premake4

    --with-sdl        : Enable SDL(GUI)
    --with-openmp     : Enable OpenMP

### Usage

Edit config.josn, then

    $ ./bin/mallie

GUI mode

* mouse left : rotate
* shift + mouse left : translate
* ctrl(or tab) + mouse left : dolly
* 'q' : quit.
* 'c' : Dump camera data to camera.dat (eye, lookat, up, quaternion)

EoL.

