cmake_minimum_required(VERSION 3.10)
project(app)
set(CMAKE_CXX_STANDARD 11)

find_package(glfw3 REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

# search for pkg-config
include (FindPkgConfig)
if (NOT PKG_CONFIG_FOUND)
    message (FATAL_ERROR "pkg-config not found")
endif ()

# check for libpng
pkg_check_modules (LIBPNG libpng16 REQUIRED)
if (NOT LIBPNG_FOUND)
    message(FATAL_ERROR "You don't seem to have libpng16 development libraries installed")
else ()
    include_directories (${LIBPNG_INCLUDE_DIRS})
    link_directories (${LIBPNG_LIBRARY_DIRS})
    link_libraries (${LIBPNG_LIBRARIES})
endif ()


set(SOURCES src/main.c src/files.h src/files.c src/mathematics.h src/mathematics.c src/meshes.h src/meshes.c src/glad.c)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

target_link_libraries(${CMAKE_PROJECT_NAME})
target_link_libraries(${CMAKE_PROJECT_NAME} glfw)