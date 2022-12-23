cmake_minimum_required(VERSION 3.23)
project(tradingsystem)

set(CMAKE_CXX_STANDARD 14)
include_directories("/opt/homebrew/Cellar/boost/1.80.0/include")
add_executable(tradingsystem main.cpp)
