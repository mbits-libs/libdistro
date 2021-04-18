if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  cmake_minimum_required (VERSION 3.10)
  project (distro CXX)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
  message(STATUS "Libdistro: Subdir")
endif()

if (MSVC)
  set(ADDITIONAL_WALL_FLAGS
      /D_CRT_SECURE_NO_WARNINGS
      /permissive-
      /W4
      /w14242
      /w14254
      /w14263
      /w14265
      /w14287
      /we4289
      /w14296
      /w14311
      /w14545
      /w14546
      /w14547
      /w14549
      /w14555
      /w14619
      /w14640
      /w14826
      /w14905
      /w14906
      /w14928
      /w14946)
else()
  set(ADDITIONAL_WALL_FLAGS
      -Wall -Wextra
      -Wnon-virtual-dtor
      -Wold-style-cast
      -Wcast-align
      -Wunused
      -Woverloaded-virtual
      -Wpedantic
      -Wconversion
      -Wsign-conversion
      -Wnull-dereference
      -Wdouble-promotion
      -Wformat=2
  )
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    list(APPEND ADDITIONAL_WALL_FLAGS -fcolor-diagnostics) # -Wlifetime
  else()
    list(APPEND ADDITIONAL_WALL_FLAGS
      -fdiagnostics-color
      -Wmisleading-indentation
      -Wduplicated-cond
      -Wduplicated-branches
      -Wlogical-op
      -Wuseless-cast
      )
  endif()
endif()

set(SRCS
    src/errors.cc
    src/package.cc
    src/regex.cc
    src/semver.cc
    src/versions.cc
    include/distro/errors.hh
    include/distro/package.hh
    include/distro/regex.hh
    include/distro/semver.hh
    include/distro/versions.hh
)

add_library(distro STATIC ${SRCS})
target_include_directories(distro PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(distro PRIVATE ${ADDITIONAL_WALL_FLAGS})
