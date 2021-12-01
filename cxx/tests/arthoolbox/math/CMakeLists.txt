add_compile_options(-g -Wall -Wextra -Wnon-virtual-dtor -Wpedantic -Wshadow)

# TEST - TOKENIZERs ###############################################################
add_executable(${PROJECT_NAME}_statistics
  test_statistics.cpp)

target_link_libraries(${PROJECT_NAME}_statistics PRIVATE gtest_main arthoolbox)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_statistics)

if(${CMAKE_VERSION} VERSION_LESS "3.10.0")
  gtest_add_tests(TARGET ${PROJECT_NAME}_statistics)
else(${CMAKE_VERSION} VERSION_LESS "3.10.0")
  gtest_discover_tests(${PROJECT_NAME}_statistics)
endif(${CMAKE_VERSION} VERSION_LESS "3.10.0")
