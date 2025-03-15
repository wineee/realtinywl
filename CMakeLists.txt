cmake_minimum_required(VERSION 3.13)

project (WayGreet
  VERSION "0.0.1"
  DESCRIPTION "Simple Greeter for greetd"
  HOMEPAGE_URL "https://github.com/wineee/WayGreet"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

include(GNUInstallDirs)

add_subdirectory(src)
