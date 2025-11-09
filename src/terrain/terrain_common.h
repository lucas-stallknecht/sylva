#pragma once

#include <daxa/utils/task_graph_types.hpp>

namespace sylva
{
    struct TerrainResources
    {
        daxa::TaskImage height_map;
        daxa::TaskImage albedo_map;
        daxa::TaskImage normal_map;
    };
} // namespace sylva
