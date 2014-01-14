-- K/FX10 target
newoption {
   trigger     = "k",
   description = "Compile for K/FX10."
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
   description = "Use SDL."
}

sources = {
   "main.cc",
   "main_console.cc",
   "main_sdl.cc",
   "mmm_io.cc",
   "render.cc",
   "camera.cc",
   "light.cc",
   "matrix.cc",
   "trackball.cc",
   "importers/tiny_obj_loader.cc",
   "importers/mesh_loader.cc",
   "bvh_accel.cc",
   "scene.cc",
   "spectrum.cc",
   "jpge.cc",
   "deps/parson/parson.c",
   "tasksys.cc",
   "texture.cc",
   "vcm.cc",
   "deps/TinyThread++-1.1/source/tinythread.cpp",
}

test_sources = {
   "test/cctest/test-atomic.cc"
}

gtest_sources = {
   "deps/gtest-1.7.0/src/gtest-all.cc",
   "deps/gtest-1.7.0/src/gtest_main.cc"
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
      files { sources }

      includedirs {
         "./",
         "deps/parson/",
         "deps/lua-5.2.2/",
         "deps/TinyThread++-1.1/source/",
      }

      links "lua"

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
            includedirs { "/Users/syoyo/work/ptex/install/include" }
            libdirs { "/Users/syoyo/work/ptex/install/lib" }
            links   { "Ptex" }
         end

      -- Windows general
      configuration { "windows" }

         includedirs { "./compat" } -- stdint

	 if _OPTIONS['with-sdl'] then
            defines { 'ENABLE_SDL' }
            includedirs { "extlibs/windows/SDL/msvc/SDL-1.2.15/include" }
            links { "SDL", "SDLmain" }
            libdirs { "extlibs/windows/SDL/msvc/SDL-1.2.15/lib/x64" }
         end

         defines { 'NOMINMAX', '_LARGEFILE_SOURCE', '_FILE_OFFSET_BITS=64' }

      -- Windows + gmake specific
      configuration { "windows", "gmake" }

         defines { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' } -- c99

         links { "stdc++", "msvcrt", "ws2_32", "winmm" }

      -- Linux specific
      configuration {"linux", "gmake"}
         defines { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' } -- c99

         if _OPTIONS['k'] then
            buildoptions { "-Xg -KPIC" } -- gcc compat, position independet code.

            -- fj openmp
            if _OPTIONS['with-openmp'] then
               buildoptions { "-Kopenmp" }
               linkoptions { "-Kopenmp" }
            end
         else
            -- gcc openmp
            if _OPTIONS['with-openmp'] then
               buildoptions { "-fopenmp" }
               linkoptions { "-fopenmp" }
            end

            -- gcc mpi
            if _OPTIONS['with-mpi'] then
               defines { 'WITH_MPI' }
            end

         end

         if _OPTIONS['with-sdl'] then
            defines { 'ENABLE_SDL' }
            buildoptions { "`sdl2-config --cflags`" }
            linkoptions { "`sdl2-config --libs`" }
         end

         if not _OPTIONS['k'] then
            linkoptions { "-pthread" }
         end

      configuration "Debug"
         defines { "DEBUG" } -- -DDEBUG
         flags { "Symbols" }
         targetdir "bin/"
         targetname "mallie_debug"

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Symbols", "Optimize" }
         if _OPTIONS['k'] then
            -- pass buildoptions { "-Kfast" }
         elseif _OPTIONS['arm'] then
            -- buildoptions { "NEON" }
         else 
            flags { "EnableSSE2" }
         end
         targetdir "bin/"
         targetname "mallie"

   -- A project defines one build target
   project "MallieTest"
      kind "ConsoleApp"
      language "C++"
      files { test_sources, gtest_sources }

      includedirs {
         "./",
         "deps/parson/",
         "deps/TinyThread++-1.1/source/",
         "deps/gtest-1.7.0/include/",
         "deps/gtest-1.7.0/",
      }

      defines { 'ENABLE_UNITTEST' }

      -- for gtest
      if _OPTIONS['k'] then
         defines { 'GTEST_HAS_TR1_TUPLE=0' }
      end

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

      -- Windows general
      configuration { "windows" }

         includedirs { "./compat" } -- stdint

         if _OPTIONS['with-sdl'] then
            defines { 'ENABLE_SDL' }
            includedirs { "extlibs/windows/SDL/msvc/SDL-1.2.15/include" }
            links { "SDL", "SDLmain" }
            libdirs { "extlibs/windows/SDL/msvc/SDL-1.2.15/lib/x64" }
         end

         defines { 'NOMINMAX', '_LARGEFILE_SOURCE', '_FILE_OFFSET_BITS=64' }

      -- Windows + gmake specific
      configuration { "windows", "gmake" }

         defines { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' } -- c99

         links { "stdc++", "msvcrt", "ws2_32", "winmm" }

      -- Linux specific
      configuration {"linux", "gmake"}
         defines { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' } -- c99

         if _OPTIONS['k'] then
            buildoptions { "-Xg -KPIC" } -- gcc compat, position independet code.

            -- fj openmp
            if _OPTIONS['with-openmp'] then
               buildoptions { "-Kopenmp" }
               linkoptions { "-Kopenmp" }
            end
         else
            -- gcc openmp
            if _OPTIONS['with-openmp'] then
               buildoptions { "-fopenmp" }
               linkoptions { "-fopenmp" }
            end
         end

         if _OPTIONS['with-sdl'] then
            defines { 'ENABLE_SDL' }
            buildoptions { "`sdl2-config --cflags`" }
            linkoptions { "`sdl2-config --libs`" }
         end

         if not _OPTIONS['k'] then
            linkoptions { "-pthread" }
         end

      configuration "Debug"
         defines { "DEBUG" } -- -DDEBUG
         flags { "Symbols" }
         targetdir "bin/"
         targetname "test_mallie_debug"

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Symbols", "Optimize" }
         if _OPTIONS['k'] then
            -- pass buildoptions { "-Kfast" }
         elseif _OPTIONS['arm'] then
            -- buildoptions { "NEON" }
         else 
            flags { "EnableSSE2" }
         end
         targetdir "bin/"
         targetname "test_mallie"
