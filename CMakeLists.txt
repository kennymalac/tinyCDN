cmake_minimum_required(VERSION 3.1)
project(TinyCDN)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_CXX_COMPILER g++-7)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Cmake 3.8 isn't out yet so we need to require c++ 17 manually
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Export a JSON file with information about our build for irony-mode
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(BASE_TEST_DIR ${PROJECT_SOURCE_DIR}/test)
# set(TINYCDN_TEST_SOURCES

# )


file(GLOB SOURCES
  "src/utility.hpp"
  "src/utility.cpp"
  "src/middlewares/file.hpp"
  "src/storedfile.h"
  "src/storedfile.cpp"
  "src/middlewares/exceptions.hpp"
  "src/middlewares/file.cpp"
  "src/master.hpp"
  "src/master.cpp"
)

#add_subdirectory(src)
# Create the run game executable
add_library(TinyCDN_Base ${SOURCES})
add_library(stdc++fs UNKNOWN IMPORTED)
set_property(TARGET stdc++fs PROPERTY IMPORTED_LOCATION "/usr/lib/gcc/x86_64-linux-gnu/7/libstdc++fs.a")
#target_link_libraries(TinyCDN_Base PUBLIC)
add_executable(Run src/main.cpp)
target_link_libraries(Run TinyCDN_Base stdc++fs)

# TODO make testing an option, see https://github.com/ComicSansMS/GhulbusBase/blob/master/CMakeLists.txt
# enable_testing()
# add_library(Catch INTERFACE)
# target_include_directories(Catch INTERFACE ${BASE_TEST_DIR}/include)

# Create the testing suite executable
# add_executable(Test ${ENGINE_TEST_SOURCES})
# target_link_libraries(Test TinyCDN_Base Catch)
# add_test(NAME TestBase COMMAND Test)

