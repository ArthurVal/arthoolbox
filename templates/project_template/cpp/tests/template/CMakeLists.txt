add_compile_options(-g -Wall -Wextra -Wnon-virtual-dtor -Wpedantic -Wshadow)

# TEST - TEMPLATE GTEST #######################################################
add_executable(${PROJECT_NAME}_gtest_template
  test_gtest_template.cpp)

target_link_libraries(${PROJECT_NAME}_gtest_template PRIVATE gtest_main)

add_dependencies(tests ${PROJECT_NAME}_gtest_template)

if(${CMAKE_VERSION} VERSION_LESS "3.10.0")
  gtest_add_tests(TARGET ${PROJECT_NAME}_gtest_template)
else(${CMAKE_VERSION} VERSION_LESS "3.10.0")
  gtest_discover_tests(${PROJECT_NAME}_gtest_template)
endif(${CMAKE_VERSION} VERSION_LESS "3.10.0")
