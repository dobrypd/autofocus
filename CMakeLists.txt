cmake_minimum_required(VERSION 3.10)
project("Autofocus" CXX)

enable_testing()

add_subdirectory(apps)
add_subdirectory(modules)

add_subdirectory(test)
