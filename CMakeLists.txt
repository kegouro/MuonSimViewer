cmake_minimum_required(VERSION 3.20)
project(MuonSimViewer VERSION 4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS
    Core Widgets Gui OpenGL OpenGLWidgets Charts PrintSupport Concurrent
)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/ComsolParser.cpp
    src/Viewport3D.cpp
    src/StatsPanel.cpp
    src/PDFExporter.cpp
    src/VideoExporter.cpp
    src/AppTheme.cpp
)

set(HEADERS
    src/MainWindow.h
    src/SimulationData.h
    src/ComsolParser.h
    src/Viewport3D.h
    src/StatsPanel.h
    src/PDFExporter.h
    src/VideoExporter.h
    src/AppTheme.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Charts
    Qt6::PrintSupport
    Qt6::Concurrent
)

# Enable high DPI support
target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_DEPRECATED_WARNINGS
)

# Optimization flags for release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -O3 -march=native)
endif()

# Install
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
