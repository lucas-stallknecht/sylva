#pragma once

#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <daxa/daxa.hpp>
#include <daxa/utils/pipeline_manager.hpp>

#include "../shader_shared/shared.inl"

namespace sylva
{
    struct TerrainInfo
    {
        std::size_t patch_grid_size;
        float patch_width;
    };

    inline std::vector<Vertex> generate_terrain_vertices(TerrainInfo const & terrain_info)
    {
        daxa_f32vec3 up_normal = {0.0f, 1.0f, 0.0f};

        std::vector<Vertex> vertices;
        vertices.reserve(terrain_info.patch_grid_size * terrain_info.patch_grid_size * 4u);

        float const total_width =
            terrain_info.patch_width * static_cast<float>(terrain_info.patch_grid_size);
        float const origin_x = -total_width * 0.5f;
        float const origin_z = -total_width * 0.5f;
        float const inv_res = 1.0f / static_cast<float>(terrain_info.patch_grid_size);

        for (std::size_t i = 0; i < terrain_info.patch_grid_size; ++i)
        {
            auto const fi = static_cast<float>(i);
            // Position patches sequentially across the total terrain width.
            float const base_x = origin_x + (fi * terrain_info.patch_width);
            float const next_x = base_x + terrain_info.patch_width;

            float const u0 = fi * inv_res;
            float const u1 = (fi + 1.0f) * inv_res;

            for (std::size_t j = 0; j < terrain_info.patch_grid_size; ++j)
            {
                auto const fj = static_cast<float>(j);
                // Position patches sequentially across the total terrain depth.
                float const base_z = origin_z + (fj * terrain_info.patch_width);
                float const next_z = base_z + terrain_info.patch_width;

                float const v0 = fj * inv_res;
                float const v1 = (fj + 1.0f) * inv_res;

                vertices.push_back({.position = {base_x, 0.0f, base_z},
                                    .uv_1 = u0,
                                    .normal = up_normal,
                                    .uv_2 = v0});
                vertices.push_back({.position = {next_x, 0.0f, base_z},
                                    .uv_1 = u1,
                                    .normal = up_normal,
                                    .uv_2 = v0});
                vertices.push_back({.position = {base_x, 0.0f, next_z},
                                    .uv_1 = u0,
                                    .normal = up_normal,
                                    .uv_2 = v1});
                vertices.push_back({.position = {next_x, 0.0f, next_z},
                                    .uv_1 = u1,
                                    .normal = up_normal,
                                    .uv_2 = v1});
            }
        }

        return vertices;
    }

}; // namespace sylva
