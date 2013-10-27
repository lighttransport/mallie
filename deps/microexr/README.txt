
-= microexr Library =-


What the hell is this library ?
-------------------------------

This library was started in order to replace the original OpenEXR
library for a limited set of capabilities. As of now, this library
will let you _write_ multi-channels OpenEXR files in scanline format,
with ZIP or ZIPS compression support, as well as EXR 2.0 deep image
files.

We choose to support only the original OpenEXR C API. We have added
one function to enable users to write multi-channel and deep EXRs so
that the library is functional (original C API only allows for RGB[A]
files).

If you are already using the interface designed in "ImfCFile.h", 
chances are that you can readily link with this library.

History and Future
-------------------

This library was written for 3Delight in order to provide simple
EXR writing routines.

Our wish to provide good reading functionalities for the most common
EXR layouts.

OpenEXR C++ API will not be supported so we will keep updating
the original C API.

Directory Structure
-------------------

README.txt

	This file.

microexr/src 

	Contains the source to the library.

microexr/test
	
	Contains simple tests that produce EXR and EXR 2.0 file.

microexr/tools

	Contains a couple useful source files but not needed for
	the library.

microexr/build

	Contains object files (.o files) after the build.

microexr/lib

	Will contain 'libmicroexr.a' after the build.

How to Build
-------------

Like this (on Linux and Mac Os X):

  cd microexr/src
  make

Windows not yet tested but as you can see, should be a simple task, for once!

To build the test:

  cd microexr/tools
  make
  ./gradient
  ./deepgradient

This generates two gradient files: "gradient.exr" and 

Dependencies
------------

We will try to keep this to a minimum. Please, no boost, no python,
no cmake or other bloat.

As of now, we need this:

  * zlib. For ZIP and ZIPS compression.
  * stl. For no good reason, could be easily removed.

