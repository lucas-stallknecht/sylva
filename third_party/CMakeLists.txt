include(FetchContent)

set(DAXA_ENABLE_UTILS_PIPELINE_MANAGER_GLSLANG on)
set(DAXA_ENABLE_UTILS_MEM on)
set(DAXA_ENABLE_UTILS_TASK_GRAPH on)
FetchContent_Declare(
    daxa
    # Temporaty
    SOURCE_DIR "C:/Users/Administrator/Documents/Daxa"
    # GIT_REPOSITORY https://github.com/Ipotrick/Daxa
    # GIT_TAG        rework-build-system
)

FetchContent_Declare(
 glm
 GIT_REPOSITORY https://github.com/g-truc/glm
 GIT_TAG 1.0.1
)

FetchContent_Declare(
 glfw
 GIT_REPOSITORY https://github.com/glfw/glfw.git
)

FetchContent_MakeAvailable(
 daxa
 glm
 glfw
)
