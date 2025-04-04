cmake_minimum_required(VERSION 3.16)

project(self-futex LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(semaphore)
target_sources(semaphore
  PUBLIC
  CountingSemaphore.h
  PRIVATE
  CountingSemaphore.cpp
)

add_executable(
  sem_tests
  tests/unit.cpp
  tests/stress.cpp
)
target_include_directories(
  sem_tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
  sem_tests
  semaphore
  GTest::gtest_main
)
