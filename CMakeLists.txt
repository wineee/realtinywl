cmake_minimum_required(VERSION 3.13)

project (tinywl
  VERSION "0.0.1"
  DESCRIPTION "dtk based markdown editor"
  HOMEPAGE_URL "https://github.com/DMarked/DMarked"
  LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

if (NOT MSVC)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
  endif()
endif()

include(GNUInstallDirs)

add_subdirectory(src)
