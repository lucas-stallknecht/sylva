#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#include "utils/terrain_patches.h"
#include "utils/grass_chunks.h"
#include "generation/terrain_generation.inl"

namespace sylva::defaults
{

    // window
    inline constexpr std::uint32_t window_width = 1600u;
    inline constexpr std::uint32_t window_height = 900u;
    inline constexpr std::uint32_t window_width_fallback = 800u;
    inline constexpr std::uint32_t window_height_fallback = 600u;

    // camera
    inline constexpr float camera_move_speed = 1.0f;
    inline constexpr float camera_mouse_sensitivity = 0.05f;
    inline constexpr float camera_fov = 50.0f;
    inline constexpr float camera_near_plane = 0.01f;
    inline constexpr float camera_far_plane = 100.0f;
    inline constexpr glm::vec3 camera_initial_position{0.0f, 0.3f, 5.0f};
    inline constexpr float camera_initial_yaw = -90.0f;
    inline constexpr float camera_initial_pitch = 0.0f;

    // terrain
    inline constexpr std::uint32_t terrain_map_resolution = 4096u;

    inline constexpr TerrainInfo terrain_info = {
        .patch_grid_size = 20,
        .patch_width = 20.0f,
    };

    inline constexpr TerrainGenerationParams terrain_generation_defaults = {
        .amplitude = 3.2f,
        .scale = 0.7f,
        .octaves = 12u,
        .persistence = 0.35f,
        .lacunarity = 1.9f,
        .slope_min = 0.42f,
        .slope_max = 0.51f,
    };

    // grass
    inline constexpr GrassChunkParams grass_chunk_params = {
        .chunk_grid_size = 5,
        .chunk_width = 1.0f,
        .blade_density = 100.0f,
    };

} // namespace sylva::defaults
