cmake_minimum_required(VERSION 3.5)
project(Monolith LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(Monolith ${SOURCES})

target_include_directories(Monolith PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(Monolith PRIVATE
    CMAKE_PROJECT_ROOT="${CMAKE_SOURCE_DIR}/"
)