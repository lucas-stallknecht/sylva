#pragma once

#include <cstddef>
#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph_types.hpp>
#include <glm/glm.hpp>

namespace sylva
{

    struct GrassChunkParams
    {
        std::size_t chunk_grid_size;
        float chunk_width;
        float blade_density;
    };

    struct GrassChunk
    {
        glm::vec3 world_origin;
        daxa::TaskBuffer blade_buffer;
    };

    inline std::shared_ptr<std::vector<GrassChunk>>
    create_grass_chunks(GrassChunkParams const & chunk_params)
    {
        std::vector<GrassChunk> chunks;
        chunks.reserve(chunk_params.chunk_grid_size * chunk_params.chunk_grid_size);

        auto const grid_size_f = static_cast<float>(chunk_params.chunk_grid_size);

        float const total_grid_width = grid_size_f * chunk_params.chunk_width;
        float const half = total_grid_width * 0.5f;

        for (std::size_t j = 0; j < chunk_params.chunk_grid_size; j++)
        {
            for (std::size_t i = 0; i < chunk_params.chunk_grid_size; i++)
            {
                GrassChunk chunk{};

                auto const i_f = static_cast<float>(i);
                auto const j_f = static_cast<float>(j);

                // Chunk origin is not at the center of the chunk
                float const world_x = -half + (i_f * chunk_params.chunk_width);
                float const world_z = -half + (j_f * chunk_params.chunk_width);

                chunk.world_origin = glm::vec3(world_x, 0.0f, world_z);

                chunks.push_back(chunk);
            }
        }

        return std::make_shared<std::vector<GrassChunk>>(std::move(chunks));
    }

}; // namespace sylva
