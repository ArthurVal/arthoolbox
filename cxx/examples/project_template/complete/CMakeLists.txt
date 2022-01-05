cmake_minimum_required(VERSION 3.11)

project(PLACEHOLDER_PROJECT_NAME
  VERSION 0.0.1
  DESCRIPTION "PLACEHOLDER_PROJECT_DESCRIPTION"
  LANGUAGES CXX)

include(CMakePrintHelpers)      #for cmake_print_variables & cmake_print_properties
include(CMakeDependentOption)   #for cmake_dependent_option

option(PLACEHOLDER_PROJECT_ACRONYM_ENABLE_BENCHMARK "Enable the benchmark build" OFF)
cmake_print_variables(PLACEHOLDER_PROJECT_ACRONYM_ENABLE_BENCHMARK)

option(PLACEHOLDER_PROJECT_ACRONYM_ENABLE_TESTING "Enable the unit tests build" OFF)
cmake_print_variables(PLACEHOLDER_PROJECT_ACRONYM_ENABLE_TESTING)

cmake_dependent_option(PLACEHOLDER_PROJECT_ACRONYM_TESTING_DL_GTEST
  "Enable the auto download of googletest lib" ON
  PLACEHOLDER_PROJECT_ACRONYM_ENABLE_TESTING OFF)
cmake_print_variables(PLACEHOLDER_PROJECT_ACRONYM_TESTING_DL_GTEST)

option(PLACEHOLDER_PROJECT_ACRONYM_ENABLE_DOC "Build the doxygen documentation" OFF)

configure_file(
  include/${PROJECT_NAME}/Config.hpp.in
  include/${PROJECT_NAME}/Config.hpp
  @ONLY)

add_subdirectory(src)

if(${PLACEHOLDER_PROJECT_ACRONYM_ENABLE_TESTING})
  enable_testing()
  add_custom_target(tests)
  add_subdirectory(tests)
endif(${PLACEHOLDER_PROJECT_ACRONYM_ENABLE_TESTING})

if(${PLACEHOLDER_PROJECT_ACRONYM_ENABLE_BENCHMARK})
  add_custom_target(benchmarks)
  add_subdirectory(benchmarks)
endif(${PLACEHOLDER_PROJECT_ACRONYM_ENABLE_BENCHMARK})

if(${PLACEHOLDER_PROJECT_ACRONYM_ENABLE_DOC})
  add_subdirectory(docs)
endif(${PLACEHOLDER_PROJECT_ACRONYM_ENABLE_DOC})
