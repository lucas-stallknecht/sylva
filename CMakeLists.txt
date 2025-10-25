cmake_minimum_required(VERSION 3.21)

project(sylva)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(third_party)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_DIR ${ROOT_DIR}/src)

add_compile_definitions(ASSETS_DIR="${CMAKE_SOURCE_DIR}/resources/assets")
add_compile_definitions(TEXTURES_DIR="${CMAKE_SOURCE_DIR}/resources/textures")

# -- Own --
file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.c??")
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC include)

# -- Dependencies --
target_link_libraries(${PROJECT_NAME} PRIVATE daxa::daxa glfw glm)
