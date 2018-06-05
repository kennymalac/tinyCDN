cmake_minimum_required(VERSION 3.1)
project(TinyCDN)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_CXX_COMPILER g++-7)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Debug)

# Cmake 3.8 isn't out yet so we need to require c++ 17 manually
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

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
)

set(TEST_SOURCES
  src/test/fileupload.cpp
  src/test/file.cpp
)

include_directories("${PROJECT_SOURCE_DIR}")

#add_subdirectory(src)
# Create the run game executable
add_library(TinyCDN_Base ${SOURCES})
add_library(stdc++fs UNKNOWN IMPORTED)
set_property(TARGET stdc++fs PROPERTY IMPORTED_LOCATION "/usr/lib/gcc/x86_64-linux-gnu/7/libstdc++fs.a")
#target_link_libraries(TinyCDN_Base PUBLIC)
add_executable(Run src/main.cpp)
target_link_libraries(Run TinyCDN_Base stdc++fs)

# TODO make testing an option, see https://github.com/ComicSansMS/GhulbusBase/blob/master/CMakeLists.txt
enable_testing()
add_library(Catch INTERFACE)

target_include_directories(Catch INTERFACE ${BASE_TEST_DIR}/include)

# Create the testing suite executable
add_executable(Test ${TEST_SOURCES})
target_link_libraries(Test TinyCDN_Base Catch stdc++fs)
add_test(NAME TestBase COMMAND Test)


#set(DOCS_OUTPUT_DIR "docs/")

#set(CLDOC "/usr/local/bin/cldoc" CACHE STRING "User specified '/path/to/cldoc'")
#if(CLDOC STREQUAL "")
#        find_program(CLDOC cldoc)
#elseif(NOT EXISTS ${CLDOC})
#        message(FATAL_ERROR "CLDOC was set to '${CLDOC}' but does not exist")
#endif()

#if(NOT DOCS_OUTPUT_DIR)
#        set(DOCS_OUTPUT_DIR "gendocs" CACHE STRING "Generated documentation output directory" FORCE)
#endif()

#if(NOT DOCS_DIR)
#        set(DOCS_DIR "" CACHE STRING "Location of external documentation files to be merged in" FORCE)
#endif()

#if(NOT ${CLDOC} STREQUAL "CLDOC-NOTFOUND")
#  if(NOT ${DOCS_DIR} STREQUAL "")
#        set(CLDOC_MERGE "--merge ${DOCS_DIR}")
#  else()
#        set(CLDOC_MERGE "")
#  endif()

#  add_custom_target(docs COMMAND
#    ${CLDOC} generate -I${PROJECT_SOURCE_DIR} std=c++17

#    --
#    ${CLDOC_MERGE}
#    --output ${DOCS_OUTPUT_DIR}
#    ${HEADERS}
#    ${SOURCES}
#    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} VERBATIM
#  )

#  set_target_properties(docs PROPERTIES
#    EXCLUDE_FROM_ALL 1
#    EXCLUDE_FROM_DEFAULT_BUILD 1
#  )

#  add_custom_target(serve_docs COMMAND
#    ${CLDOC} serve -- ${DOCS_OUTPUT_DIR}
#    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#  )

#  set_target_properties(docs PROPERTIES
#    EXCLUDE_FROM_ALL 1
#    EXCLUDE_FROM_DEFAULT_BUILD 1
#  )

#  add_dependencies(serve_docs docs)
#else()
#  message(STATUS
#    "Documentation will not be generated"
#  )
#endif()

