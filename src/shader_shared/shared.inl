#pragma once

#include <daxa/daxa.inl>

struct CamInfo
{
    daxa_f32vec3 position;
    daxa_f32mat4x4 proj_view;
};
DAXA_DECL_BUFFER_PTR(CamInfo)

struct Vertex
{
    daxa_f32vec3 position;
    daxa_f32 uv_1;
    daxa_f32vec3 normal;
    daxa_f32 uv_2;
};
DAXA_DECL_BUFFER_PTR(Vertex)

struct GrassBlade
{
    daxa_f32vec3 position;
};
DAXA_DECL_BUFFER_PTR(GrassBlade)
