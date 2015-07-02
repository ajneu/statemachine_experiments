cmake_minimum_required(VERSION 3.2)

project(ping_pong)

include(${PROJECT_SOURCE_DIR}/cmake_lib_hints.txt)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

set(target ping_pong)
set(src ping_pong.cpp)

find_package(Boost COMPONENTS system) # thread)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  set(libs ${libs} ${Boost_LIBRARIES})
else()
  message()
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads)
set(libs ${libs} ${CMAKE_THREAD_LIBS_INIT})

add_executable(${target} ${src})
target_link_libraries(${target} ${libs})
