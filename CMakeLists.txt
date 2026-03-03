cmake_minimum_required(VERSION 3.16)

project(PhotoOrganizerQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(PhotoOrganizerQt
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/photolibrary.h
    src/photolibrary.cpp
)

target_link_libraries(PhotoOrganizerQt PRIVATE Qt6::Widgets)

if(WIN32)
    set_target_properties(PhotoOrganizerQt PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
