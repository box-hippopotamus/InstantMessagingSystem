cmake_minimum_required(VERSION 3.28.3)

project(IMSystem)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/speech)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/file)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/user)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/transmit)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/message)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/friend)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/gateway)

# set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})