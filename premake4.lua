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
   "matrix.cc",
   "trackball.cc",
   "importers/tiny_obj_loader.cc",
   "importers/mesh_loader.cc",
   "bvh_accel.cc",
   "scene.cc",
   "jpge.cc"
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
         "./"
      }

      -- MacOSX. Guess we use gcc.
      configuration { "macosx", "gmake" }

         defines { '_LARGEFILE_SOURCE', '_FILE_OFFSET_BITS=64' }

         -- SDL
         if _OPTIONS['with-sdl'] then
            defines { 'ENABLE_SDL' }
            buildoptions { "`sdl-config --cflags`" }
            buildoptions { "-msse2" }
            linkoptions { "`sdl-config --libs`" }
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

         -- gcc openmp
         if _OPTIONS['with-openmp'] then
            buildoptions { "-fopenmp" }
            linkoptions { "-fopenmp" }
         end

         if _OPTIONS['with-sdl'] then
            defines { 'ENABLE_SDL' }
            buildoptions { "`sdl-config --cflags`" }
            linkoptions { "`sdl-config --libs`" }
         end

         linkoptions { "-pthread" }

      configuration "Debug"
         defines { "DEBUG" } -- -DDEBUG
         flags { "Symbols" }
         targetname "mallie_debug"

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Symbols", "Optimize", "EnableSSE2" }
         targetname "mallie"
