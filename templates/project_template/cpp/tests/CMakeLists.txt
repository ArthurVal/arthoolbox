project(test_${PROJECT_NAME})

message(STATUS "-- Build tests have been enabled")

# GTest AUTO-DL ###############################################################
if(${PLACEHOLDER_PROJECT_ACRONYM_TESTING_DL_GTEST})
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

  message(STATUS "GTest Source dir: ${googletest_SOURCE_DIR}")
  message(STATUS "GTest Bin dir: ${googletest_BINARY_DIR}")

  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
  include(GoogleTest)
endif(${PLACEHOLDER_PROJECT_ACRONYM_TESTING_DL_GTEST})

# TEST - TEMPLATE #############################################################
add_subdirectory(template)
