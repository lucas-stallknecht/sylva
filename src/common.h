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
        daxa::BufferId vertex_buffer_id;
        std::size_t vertex_count = 0;
    };

    struct GrassChunk
    {
        glm::vec3 world_origin;
        daxa_u32 seed;
        daxa::BufferId blade_buffer_id;
        std::size_t blade_count = 0;
    };

    struct Scene
    {
        std::shared_ptr<std::vector<GrassChunk>> grass_chunks;
        daxa::TaskImage terrain_height_map;
        daxa::TaskImage terrain_albedo_map;
        daxa::TaskImage terrain_normal_map;
        daxa::TaskBuffer grass_blades_buffer;
    };

} // namespace sylva
