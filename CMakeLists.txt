cmake_minimum_required(VERSION 3.15)
project(RibbonFu)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_PREFIX_PATH $ENV{QT_DIR}/$ENV{QT_VERSION}/gcc_64/lib/cmake)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)

aux_source_directory(. DIRS_SRCS)

add_executable(RibbonFu main.cpp FramelessHelper.cpp FramelessHelper.h)
target_link_libraries(RibbonFu Qt5::Widgets)