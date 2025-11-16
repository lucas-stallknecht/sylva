#pragma once

#include <daxa/daxa.inl>

struct CamInfo
{
    daxa_f32vec3 position;
    daxa_f32mat4x4 proj_view;
};
DAXA_DECL_BUFFER_PTR(CamInfo)
