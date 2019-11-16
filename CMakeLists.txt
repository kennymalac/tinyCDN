cmake_minimum_required(VERSION 3.3)
project(TinyCDN)
set(PROJECT_VERSION 0.1.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Debug)

# Cmake 3.8 isn't out yet so we need to require c++ 17 manually
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

# Export a JSON file with information about our build for irony-mode
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(BASE_UNIT_TEST_DIR ${PROJECT_SOURCE_DIR}/test)
set(BASE_INTEGRATION_TEST_DIR ${PROJECT_SOURCE_DIR}/test/integration-tests)

# set(TINYCDN_TEST_SOURCES

# )

set(PROJECT_SOURCE_DIR, "src/")

# add_subdirectory("src/")

set(SOURCES
  src/utility.hpp
  src/utility.cpp
  src/hashing.hpp
  src/middlewares/file.hpp
  src/middlewares/FileStorage/storedfile.hpp
  src/middlewares/FileStorage/storedfile.cpp
  src/middlewares/exceptions.hpp
  src/middlewares/exceptions.cpp
  src/middlewares/file.cpp
  src/middlewares/FileStorage/storage.hpp
  src/middlewares/FileStorage/storage.hpp
  src/middlewares/FileStorage/storage.cpp
#  src/middlewares/FileStorage/haystack.hpp
#  src/middlewares/FileStorage/haystack.cpp
  src/middlewares/FileStorage/filesystem.hpp
  src/middlewares/FileStorage/filesystem.cpp
  src/middlewares/Volume/marshaller.hpp
  src/middlewares/Volume/volume.hpp
  src/middlewares/Volume/volume.cpp
  src/middlewares/Volume/services.hpp
  src/middlewares/StorageCluster/requestTypes.hpp
  src/middlewares/StorageCluster/requestParser.hpp
  src/middlewares/StorageCluster/responseTypes.hpp
  src/middlewares/StorageCluster/responseParser.hpp
  src/middlewares/StorageCluster/services.hpp
  src/middlewares/StorageCluster/marshaller.hpp
  src/middlewares/StorageCluster/storagecluster.hpp
  src/middlewares/StorageCluster/storagecluster.cpp
  src/middlewares/StorageCluster/session.hpp
  src/middlewares/StorageCluster/session.cpp
  src/middlewares/Master/requestTypes.hpp
  src/middlewares/Master/requestParser.hpp
  src/middlewares/Master/responseTypes.hpp
  src/middlewares/Master/responseParser.hpp
  src/middlewares/Master/master.hpp
  src/middlewares/Master/master.cpp
  # src/middlewares/Client/services.hpp
  # src/middlewares/Client/services.cpp
  # src/middlewares/Client/client.h
  # src/middlewares/Client/client.cpp
)

set(UNIT_TEST_SOURCES
  src/test/master.cpp
  src/test/hashing.cpp
  src/test/volume.cpp
)

set(INTEGRATION_TEST_SOURCES
#  src/test/integration-tests/fileupload.cpp
#  src/test/integration-tests/filehosting.cpp
  src/test/integration-tests/file.cpp
)

include_directories("${PROJECT_SOURCE_DIR}")

#add_subdirectory(src)
include_directories(/usr/local/include/asio)
add_library(TinyCDN_Base SHARED ${SOURCES})
add_library(stdc++fs UNKNOWN IMPORTED)
set_property(TARGET stdc++fs PROPERTY IMPORTED_LOCATION "/usr/lib/gcc/x86_64-linux-gnu/8/libstdc++fs.a")

#target_link_libraries(TinyCDN_Base PUBLIC)
add_executable(Run src/main.cpp)
target_link_libraries(TinyCDN_Base stdc++fs)
target_link_libraries(Run TinyCDN_Base stdc++fs)



# TODO make testing an option, see https://github.com/ComicSansMS/GhulbusBase/blob/master/CMakeLists.txt
enable_testing()
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${BASE_UNIT_TEST_DIR}/include)

option(BUILD_UNIT_TESTS "Compile unit tests" ON)
if(BUILD_INTEGRATION_TESTS)
 # Create the testing suite executable
  add_executable(UnitTests ${UNIT_TEST_SOURCES})
  target_link_libraries(UnitTests TinyCDN_Base Catch stdc++fs)
  add_test(NAME UnitTestsBase COMMAND UnitTests)
endif()

option(BUILD_INTEGRATION_TESTS "Compile integration tests" ON)
if(BUILD_INTEGRATION_TESTS)
  # Create the testing suite executable
  add_executable(IntegrationTests ${INTEGRATION_TEST_SOURCES})
  target_link_libraries(IntegrationTests TinyCDN_Base Catch stdc++fs)
  add_test(NAME IntegrationTestsBase COMMAND IntegrationTests)
endif()

option(BUILD_CFFI "Generate the shared library" OFF)
if(BUILD_CFFI)
  add_library(tinyCDN SHARED src/middlewares/Client/client.h src/middlewares/Client/client.c src/middlewares/Client/client.cpp)
  add_executable(TestInterop src/middlewares/Client/client.cpp)

  target_link_libraries(TestInterop tinyCDN)
  set_target_properties(tinyCDN PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "CXX")
  target_link_libraries(tinyCDN TinyCDN_Base)
endif()

option(BUILD_DOCUMENTATION "Use Doxygen to create the HTML based API documentation" ON)
if(BUILD_DOCUMENTATION)
    set(DOXYGEN_EXCLUDE_PATTERNS catch.hpp */include/pistache/* */include/asio-1.12.2/*)
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

