#include <daxa/daxa.inl>

#extension GL_EXT_debug_printf : enable

#include "opaque.inl"
#include "../shader_shared/random.glsl"

DAXA_DECL_PUSH_CONSTANT(DrawGrassBladesPush, push)

#if DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_VERTEX

layout(location = 1) out vec3 v_color;

void main() {
    Vertex vert = deref_i(push.vertex_buffer, gl_VertexIndex);
    GrassBlade blade = deref_i(push.blade_buffer, gl_InstanceIndex);

    vec4 worldPos = vec4(blade.position + vert.position, 1);

    gl_Position = deref(push.attachments.camera).proj_view * worldPos;
    v_color = random_color(push.chunk_seed);
    // v_color = blade.color;
}

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_FRAGMENT

layout(location = 1) in vec3 v_color;
layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(v_color, 1.0);
}

#endif
