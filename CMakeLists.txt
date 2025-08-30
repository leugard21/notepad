cmake_minimum_required(VERSION 3.31)
project(notepad VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(notepad
  src/main.cpp
  src/App.cpp
  src/MainWindow.cpp
  src/EditorWidget.cpp
  include/App.h
  include/MainWindow.h
  include/EditorWidget.h
  src/Highlighter.cpp
  include/Highlighter.h
)

target_include_directories(notepad PRIVATE include)
target_link_libraries(notepad PRIVATE Qt6::Widgets)

if(WIN32)
  set_property(TARGET notepad PROPERTY WIN32_EXECUTABLE TRUE)
endif()
