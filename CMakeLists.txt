cmake_minimum_required(VERSION 3.16)
project(Simulador-de-Processador-RISC-V CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
# -------------------------

set(PROJECT_SOURCES
        src/main.cpp
        src/core/Core.cpp
        src/cache/Cache.cpp
        src/gui/mainwindow.cpp
)

set(PROJECT_HEADERS
        src/core/Core.h
        src/core/RegistradoresPipeline.h
        src/cache/Cache.h
        src/gui/mainwindow.h
)

set(PROJECT_UI_FILES
        src/gui/mainwindow.ui
        src/core/Instruction.h
        src/core/RegistradoresPipeline.h
        src/core/TiposPipeline.h
)

add_executable(Simulador-de-Processador-RISC-V
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
        ${PROJECT_UI_FILES}
        src/gui/launcherwindow.h src/gui/launcherwindow.cpp src/gui/launcherwindow.ui
        src/gui/demowindow.h src/gui/demowindow.cpp src/gui/demowindow.ui
)

target_link_libraries(Simulador-de-Processador-RISC-V PRIVATE Qt6::Widgets)

target_include_directories(Simulador-de-Processador-RISC-V PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
