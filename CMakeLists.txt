cmake_minimum_required(VERSION 3.17)
project(myfind)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost COMPONENTS filesystem program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
add_executable(myfind myfind.cpp)
target_link_libraries(myfind ${Boost_LIBRARIES})
