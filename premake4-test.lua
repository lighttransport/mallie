   -- A project building unit test
   project "MallieTest"
      kind "ConsoleApp"
      language "C++"

      files { gtest_sources, sources, test_sources, tinyjs_sources }

      includedirs {
         "./",
         "deps/gtest-1.7.0/include",
         "deps/gtest-1.7.0/",
         "deps/parson/",
         "deps/TinyThread++-1.1/source/",
         "deps/tinyjs/",
      }

      --   defines { 'GTEST_HAS_TR1_TUPLE=0' }

      -- MacOSX. Guess we use gcc.
      configuration { "macosx", "gmake" }

         -- gcc openmp
         if _OPTIONS['with-openmp'] then
            buildoptions { "-fopenmp" }
            linkoptions { "-fopenmp" }
         end

      -- Windows specific. Assumes visual studio.
      configuration { "windows" }

         -- includedirs { './compat' } -- stdint.h 
         defines { 'NOMINMAX', '_LARGEFILE_SOURCE', '_FILE_OFFSET_BITS=64' }
         defines { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' } -- c99

      -- Linux specific
      configuration { "linux" }

         links { "dl" }
         links { "pthread" }

         defines { '__STDC_CONSTANT_MACROS', '__STDC_LIMIT_MACROS' } -- c99
         defines { '_LARGEFILE_SOURCE', '_FILE_OFFSET_BITS=64' }

         -- gcc openmp
         if _OPTIONS['with-openmp'] then
            buildoptions { "-fopenmp" }
            linkoptions { "-fopenmp" }
         end

      configuration "Debug"
         defines { "DEBUG" } -- -DDEBUG
         flags { "Symbols" }

         targetname "test_mallie_d"

      configuration "Release"
         flags { "Symbols", "Optimize" }

         targetname "test_mallie"
