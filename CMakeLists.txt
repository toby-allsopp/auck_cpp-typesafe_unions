cmake_minimum_required(VERSION 3.2)
project(StaticUnions)

add_subdirectory(spdlog)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads)

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -pedantic)
endif()

add_executable(tmp tmp.cpp svu.cpp)
target_link_libraries(tmp spdlog ${CMAKE_THREAD_LIBS_INIT})

add_executable(test_multivisitor test_multivisitor.cpp)
target_link_libraries(test_multivisitor spdlog ${CMAKE_THREAD_LIBS_INIT})
