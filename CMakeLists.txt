cmake_minimum_required(VERSION 3.14)

project(testeng VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(Qt5 COMPONENTS Core Quick REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        QVTKFrameBufferObjectItem.cpp
        qml.qrc
)

add_executable(testeng ${PROJECT_SOURCES})

target_link_libraries(testeng PRIVATE
    Qt5::Core
    Qt5::Quick
    ${VTK_LIBRARIES}
)
