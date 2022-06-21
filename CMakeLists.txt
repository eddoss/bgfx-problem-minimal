cmake_minimum_required(VERSION 3.19)
project(bgfx-minimal LANGUAGES CXX)

add_executable(${PROJECT_NAME} main.cpp)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD                17
        CXX_STANDARD_REQUIRED       YES
        CXX_EXTENSIONS              NO
        RUNTIME_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/lib"
)

find_package(bgfx CONFIG REQUIRED)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/src"
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        bgfx::bgfx
)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        ROOTDIR=L"${CMAKE_CURRENT_LIST_DIR}"
)