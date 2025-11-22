#include <daxa/daxa.inl>

#extension GL_EXT_debug_printf : enable

#include "grass_rendering.inl"

DAXA_DECL_PUSH_CONSTANT(RenderGrassPush, push)

#if DAXA_SHADER_STAGE == DAXA_SHHADER_STAGE_VERTEX

void main() {
    Vertex vert = deref_i(push.attachments.vertices, gl_VertexIndex);

    gl_Position = vec4(vert.position, 1);
}

#elif DAXA_SHADER_STAGE == DAXA_SHHADER_STAGE_FRAGMENT

layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(1.0);
}

#endif
