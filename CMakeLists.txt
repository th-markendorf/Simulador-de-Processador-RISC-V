cmake_minimum_required(VERSION 4.0)
project(Simulador-de-Processador-RISC-V CXX)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(Simulador-de-Processador-RISC-V ${SOURCES}
        src/core/Core.cpp
        src/core/Core.h)
