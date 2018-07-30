cmake_minimum_required(VERSION 3.3)
project(TinyCDN)
set(PROJECT_VERSION 0.0.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_CXX_COMPILER g++-7)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Debug)

# Cmake 3.8 isn't out yet so we need to require c++ 17 manually
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

# Export a JSON file with information about our build for irony-mode
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(BASE_TEST_DIR ${PROJECT_SOURCE_DIR}/test)
# set(TINYCDN_TEST_SOURCES

# )

set(PROJECT_SOURCE_DIR, "src/")

# add_subdirectory("src/")

set(SOURCES
  src/utility.hpp
  src/utility.cpp
  src/middlewares/file.hpp
  src/middlewares/FileStorage/storedfile.hpp
  src/middlewares/FileStorage/storedfile.cpp
  src/middlewares/exceptions.hpp
  src/middlewares/exceptions.cpp
  src/middlewares/file.cpp
  src/middlewares/FileStorage/storage.hpp
  src/middlewares/FileStorage/storage.cpp
  src/middlewares/FileStorage/haystack.hpp
  src/middlewares/FileStorage/haystack.cpp
  src/middlewares/FileStorage/filesystem.hpp
  src/middlewares/FileStorage/filesystem.cpp
  src/master.hpp
  src/master.cpp
  src/middlewares/file_interop.h
  src/interop.cpp
)

set(TEST_SOURCES
  src/test/fileupload.cpp
  src/test/filehosting.cpp
  src/test/file.cpp
)

include_directories("${PROJECT_SOURCE_DIR}")

#add_subdirectory(src)
add_library(TinyCDN_Base SHARED ${SOURCES})
add_library(stdc++fs UNKNOWN IMPORTED)
set_property(TARGET stdc++fs PROPERTY IMPORTED_LOCATION "/usr/lib/gcc/x86_64-linux-gnu/7/libstdc++fs.a")
#target_link_libraries(TinyCDN_Base PUBLIC)
add_executable(Run src/main.cpp)
target_link_libraries(TinyCDN_Base stdc++fs)
target_link_libraries(Run TinyCDN_Base stdc++fs)

# TODO make testing an option, see https://github.com/ComicSansMS/GhulbusBase/blob/master/CMakeLists.txt
enable_testing()
add_library(Catch INTERFACE)

target_include_directories(Catch INTERFACE ${BASE_TEST_DIR}/include)

# Create the testing suite executable
add_executable(Test ${TEST_SOURCES})
target_link_libraries(Test TinyCDN_Base Catch stdc++fs)
add_test(NAME TestBase COMMAND Test)


option(BUILD_CFFI "Generate the shared library" ON)
if(BUILD_CFFI)
  add_library(tinyCDN SHARED src/middlewares/file_interop.h src/interop.c src/interop.cpp)
  add_executable(TestInterop src/interop.cpp)

  target_link_libraries(TestInterop tinyCDN)
  set_target_properties(tinyCDN PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "CXX")
  target_link_libraries(tinyCDN TinyCDN_Base)
endif()

option(BUILD_DOCUMENTATION "Use Doxygen to create the HTML based API documentation" OFF)
if(BUILD_DOCUMENTATION)
    set(DOXYGEN_EXCLUDE_PATTERNS catch.hpp */include/pistache/*)
    set(DOXYGEN_EXCLUDE_DIR)
    set(DOXYGEN_OUTPUT_DIRECTORY "./docs")

    find_package(Doxygen
        REQUIRED dot
        OPTIONAL_COMPONENTS mscgen dia)

    doxygen_add_docs(
        doxygen
        ${PROJECT_SOURCE_DIR}
        COMMENT "Generate documentation")
endif()

