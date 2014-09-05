-- Unit test
newoption {
   trigger     = "with-unittest",
   description = "Build unit test."
}

-- ARM target
newoption {
   trigger     = "arm",
   description = "Compile for ARM."
}

-- OpenMP
newoption {
   trigger     = "with-openmp",
   description = "Use OpenMP."
}

-- MPI
newoption {
   trigger     = "with-mpi",
   description = "Use MPI."
}

-- Ptex
newoption {
   trigger     = "with-ptex",
   description = "Use Ptex library."
}

-- FITS
newoption {
   trigger     = "with-cfitsio",
   description = "Use C-binding of FITS IO."
}

-- SDL
newoption {
   trigger     = "with-sdl",
   description = "Use SDL2.0."
}

main_sources = {
   "main.cc",
   "main_console.cc",
   "main_sdl.cc",
}

sources = {
   "mmm_io.cc",
   "render.cc",
   "camera.cc",
   "light.cc",
   "matrix.cc",
   "trackball.cc",
   "importers/tiny_obj_loader.cc",
   "importers/tiny_obj_loader.h",
   "importers/eson.cc",
   "importers/eson.h",
   "importers/mesh_loader.cc",
   "importers/mesh_loader.h",
   "importers/mmd_scene.cc",
   "importers/mmd_scene.h",
   "importers/mmd_math.h",
   "importers/vmd_animation.cc",
   "importers/vmd_animation.h",
   "importers/vmd_reader.cc",
   "importers/vmd_reader.h",
   "importers/pmd_reader.cc",
   "importers/pmd_reader.h",
   "importers/pmd_model.h",
   "bvh_accel.cc",
   "scene.cc",
   "spectrum.cc",
   "jpge.cc",
   "deps/parson/parson.c",
   "tasksys.cc",
   "texture.cc",
   "vcm.cc",
   "script_engine.cc",
   "deps/TinyThread++-1.1/source/tinythread.cpp",
   "miniexr.cpp",
   "tinyexr.cc",
}

tinyjs_sources = {
   "deps/tinyjs/TinyJS.cpp",
   "deps/tinyjs/TinyJS_Functions.cpp",
   "deps/tinyjs/TinyJS_MathFunctions.cpp",
}

test_sources = {
   "test/cctest/test-atomic.cc",
   "test/cctest/test-main.cc"
}

gtest_sources = {
   "deps/gtest-1.7.0/src/gtest-all.cc",
   -- "deps/gtest-1.7.0/src/gtest_main.cc"
}


newaction {
   trigger     = "install",
   description = "Install the software",
   execute = function ()
      -- copy files, etc. here
   end
}

-- premake4.lua
solution "MallieSolution"
   configurations { "Release", "Debug" }

   if (os.is("windows")) then
      platforms { "native", "x32", "x64" }
   else
      platforms { "native", "x32", "x64" }
   end

   -- A project defines one build target
   project "Mallie"
      kind "ConsoleApp"
      language "C++"

      files { main_sources, sources, tinyjs_sources }

      includedirs {
         "./",
         "deps/parson/",
         "deps/TinyThread++-1.1/source/",
         "deps/tinyjs/",
      }

      -- MacOSX. Guess we use gcc.
      configuration { "macosx", "gmake" }

         defines { '_LARGEFILE_SOURCE', '_FILE_OFFSET_BITS=64' }

         -- SDL
         if _OPTIONS['with-sdl'] then
            defines { 'ENABLE_SDL' }
            buildoptions { "`sdl2-config --cflags`" }
            buildoptions { "-msse2" }
            linkoptions { "`sdl2-config --libs`" }
         end

         -- gcc openmp
         if _OPTIONS['with-openmp'] then
            buildoptions { "-fopenmp" }
            linkoptions { "-fopenmp" }
         end

         -- gcc mpi
         if _OPTIONS['with-mpi'] then
            defines { 'WITH_MPI' }
         end

         -- Ptex
         if _OPTIONS['with-ptex'] then
            defines { 'ENABLE_PTEX' }
            includedirs { "./deps/ptex-master/install/include" }
            libdirs { "./deps/ptex-master/install/lib" }
            links   { "Ptex" }
         end

      -- Windows general
      configuration { "windows" }

         includedirs { "./compat" } -- stdint

         if _OPTIONS['with-sdl'] then
            defines { 'ENABLE_SDL' }
            includedirs { "extlibs/windows/SDL2-2.0.3/include/" }
            links { "SDL2", "SDL2main" }
            libdirs { "extlibs/windows/SDL2-2.0.3/lib/x64/" }
         end

         defines { '_USE_MATH_DEFINES' }
         defines { 'NOMINMAX', '_LARGEFILE_SOURCE', '_FILE_OFFSET_BITS=64' }

      -- Windows + gmake specific
      configuration { "windows", "gmake" }

         defines { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' } -- c99

         links { "stdc++", "msvcrt", "ws2_32", "winmm" }

      -- Linux specific
      configuration {"linux", "gmake"}
         defines { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' } -- c99

         -- gcc openmp
         if _OPTIONS['with-openmp'] then
            buildoptions { "-fopenmp" }
            linkoptions { "-fopenmp" }
         end

         -- gcc mpi
         if _OPTIONS['with-mpi'] then
            defines { 'WITH_MPI' }
         end

         -- Ptex
         if _OPTIONS['with-ptex'] then
            defines { 'ENABLE_PTEX' }
            includedirs { "./deps/ptex-master/install/include" }
            libdirs { "./deps/ptex-master/install/lib" }
            links   { "Ptex" }
         end

         if _OPTIONS['with-sdl'] then
            defines { 'ENABLE_SDL' }
            buildoptions { "`sdl2-config --cflags`" }
            linkoptions { "`sdl2-config --libs`" }
         end

      configuration "Debug"
         defines { "DEBUG" } -- -DDEBUG
         flags { "Symbols" }
         targetdir "bin/"
         targetname "mallie_debug"

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Symbols", "Optimize" }
         if _OPTIONS['arm'] then
            -- buildoptions { "NEON" }
         else 
            flags { "EnableSSE2" }
         end
         targetdir "bin/"
         targetname "mallie"

   if _OPTIONS['with-unittest'] then
      dofile "premake4-test.lua"
   end

