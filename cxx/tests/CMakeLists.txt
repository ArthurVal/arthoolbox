project(${PROJECT_NAME}_tests)

add_custom_target(${PROJECT_NAME})

add_dependencies(tests ${PROJECT_NAME})

add_subdirectory(arthoolbox)
