cmake_minimum_required(VERSION 3.20)
project(universal_julia_wrapper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

set(UNIVERSAL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/universal/include/sw")

add_library(universal SHARED src/libuniversal_wrapper.cpp)
target_include_directories(universal BEFORE PRIVATE ${UNIVERSAL_INCLUDE_DIR})
set_target_properties(universal PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Install target required by BinaryBuilder (RUNTIME covers .dll on Windows)
install(TARGETS universal
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
