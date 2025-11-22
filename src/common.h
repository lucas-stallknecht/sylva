#pragma once

#include <glm/glm.hpp>
#include <daxa/utils/task_graph_types.hpp>

namespace sylva
{

    struct Vertex
    {
        glm::vec3 position{};
        float uv_1{};
        glm::vec3 normal{};
        float uv_2{};
    };

    struct Geometry
    {
        daxa::TaskBuffer vertex_buffer;
        std::size_t vertex_count = 0;
    };

    struct TerrainResources
    {
        daxa::TaskImage height_map;
        daxa::TaskImage albedo_map;
        daxa::TaskImage normal_map;
    };

} // namespace sylva
