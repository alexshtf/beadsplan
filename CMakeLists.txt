cmake_minimum_required(VERSION 2.8.12)
project(beadsplan)
find_package(Qt5Widgets)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_definitions(-DGCO_ENERGYTYPE=double)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(GLOB gco_cpp gco/*.h)
file(GLOB gco_headers gco/*.cpp)
add_library(graph_cut_optimization OBJECT ${gco_cpp} ${gco_headers})


set(CMAKE_AUTOMOC ON)
set(SOURCE_FILES main.cpp MainWindow.cpp MainWindow.h)
set(FORM_FILES MainWindow.ui)
qt5_wrap_ui(FORM_HEADERS ${FORM_FILES})
add_executable(beadsplan ${SOURCE_FILES} ${FORM_HEADERS} $<TARGET_OBJECTS:graph_cut_optimization>)


qt5_use_modules(beadsplan Widgets)
