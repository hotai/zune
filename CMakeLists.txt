cmake_minimum_required(VERSION 3.20)

project(zune)

set(CMAKE_CXX_STANDARD 23)

add_executable(zune tokenizer.cpp parser.cpp assemble.cpp main.cpp)
