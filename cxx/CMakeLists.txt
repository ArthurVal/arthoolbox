cmake_minimum_required(VERSION 3.9)

project(arthoolbox
  VERSION 0.1.0
  DESCRIPTION "Personnal project containing general purpose usefull code snippets"
  LANGUAGES CXX)

include(CMakePrintHelpers)      #for cmake_print_variables & cmake_print_properties
include(CMakeDependentOption)   #for cmake_dependent_option

###############################################################################
#                                GLOBAL OPTIONS                               #
###############################################################################
option(ARTBX_ENABLE_TESTING "Enable the unit tests" OFF)
cmake_print_variables(ARTBX_ENABLE_TESTING)

cmake_dependent_option(ARTBX_DOWNLOAD_GTEST "Enable the auto download of googletest lib" ON
  ARTBX_ENABLE_TESTING OFF)
cmake_print_variables(ARTBX_DOWNLOAD_GTEST)

option(ARTBX_ENABLE_BENCHMARK "Enable the benchmark builds" OFF)
cmake_print_variables(ARTBX_ENABLE_BENCHMARK)

add_subdirectory(src)

# TESTS DEPENDENCIES ##########################################################
if(${ARTBX_ENABLE_TESTING})
  message(STATUS "Tests have been enabled")

  if(${ARTBX_DOWNLOAD_GTEST})
    message(STATUS "Automatic GTest download requested: ...")
    if(${CMAKE_VERSION} VERSION_LESS 3.11)
      message(WARNING
        "Can't download GTest automatically.\n"
        "CMake Version (${CMAKE_VERSION}) needs to be higher or equal than 3.11.")

      message(STATUS "Automatic GTest download request: ABORTED")

    else(${CMAKE_VERSION} VERSION_LESS 3.11)

      include(FetchContent)

      FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.11.0
        GIT_PROGRESS ON)

      FetchContent_GetProperties(googletest)

      if(NOT ${googletest_POPULATED})
        message(STATUS "Populating googletest:...")
        FetchContent_Populate(googletest)
        message(STATUS "Populating googletest: OK")

      endif(NOT ${googletest_POPULATED})

      message(STATUS "Automatic GTest download request: OK")

    endif(${CMAKE_VERSION} VERSION_LESS 3.11)
  endif(${ARTBX_DOWNLOAD_GTEST})

  message(STATUS "GTest Source dir: ${googletest_SOURCE_DIR}")
  message(STATUS "GTest Bin dir: ${googletest_BINARY_DIR}")

  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
  include(GoogleTest)

  # This target is only used as a meta target to build all the tests
  add_custom_target(tests)

  enable_testing()
  
  add_subdirectory(tests)
endif(${ARTBX_ENABLE_TESTING})

if(${ARTBX_ENABLE_BENCHMARK})
  add_subdirectory(benchmarks)
endif(${ARTBX_ENABLE_BENCHMARK})
