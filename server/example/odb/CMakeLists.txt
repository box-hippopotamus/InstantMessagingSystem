cmake_minimum_required(VERSION 3.28.3)

project(odb-test-all)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test2)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
