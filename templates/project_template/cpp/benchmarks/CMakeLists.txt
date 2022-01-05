project(${PROJECT_NAME}_benchmark)

message(STATUS "-- Benchmarks have been enabled")
find_package(benchmark REQUIRED)

add_compile_options(-O3)

add_executable(${PROJECT_NAME}_template bench_template.cpp)
target_link_libraries(${PROJECT_NAME}_template benchmark::benchmark)
