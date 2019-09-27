cmake_minimum_required(VERSION 3.2)
project(bytecs_tests)

add_executable(bytecs_base_test Tests.cpp)
add_executable(bytecs_benchmark benchmark.cpp)
add_executable(bytecs_benchmark_entt benchmark_entt.cpp)