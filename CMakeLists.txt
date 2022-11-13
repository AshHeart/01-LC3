cmake_minimum_required(VERSION 3.10)

project(lc3 VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(LC3 LC3.cpp)

add_subdirectory(AS)
list(APPEND EXTRA_LIBS AS)

target_link_libraries(LC3 PUBLIC ${EXTRA_LIBS})

target_include_directories(LC3 PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )
