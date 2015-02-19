cmake_minimum_required(VERSION 2.8.12)
project(beadsplan)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp MainWindow.cpp MainWindow.h)
set(FORM_FILES MainWindow.ui)
qt5_wrap_ui(FORM_HEADERS ${FORM_FILES})

add_executable(beadsplan ${SOURCE_FILES} ${FORM_HEADERS})
qt5_use_modules(beadsplan Widgets)
