cmake_minimum_required(VERSION 3.10)
project(SyslogViewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${GTKMM_INCLUDE_DIRS}
)

link_directories(
    ${GTKMM_LIBRARY_DIRS}
)

add_definitions(
    ${GTKMM_CFLAGS_OTHER}
)

# Source files
set(SOURCES
    src/syslog_message.cpp
    src/udp_listener.cpp
    src/syslog_dialog.cpp
)

# Library for reusable dialog
add_library(syslog_dialog_lib STATIC ${SOURCES})
target_link_libraries(syslog_dialog_lib
    ${GTKMM_LIBRARIES}
    pthread
)

# Standalone executable
add_executable(syslog_viewer
    src/main.cpp
)

target_link_libraries(syslog_viewer
    syslog_dialog_lib
    ${GTKMM_LIBRARIES}
    pthread
)

# Installation
install(TARGETS syslog_viewer DESTINATION bin)
install(FILES include/syslog_dialog.h DESTINATION include/syslog_viewer)
install(TARGETS syslog_dialog_lib DESTINATION lib)
