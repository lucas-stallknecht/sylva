#include <daxa/daxa.inl>

#define UP vec3(0.0, 1.0, 0.0)

#extension GL_EXT_debug_printf : enable

#include "opaque.inl"
#include "../shader_shared/random.glsl"
#include "../shader_shared/bezier.glsl"

DAXA_DECL_PUSH_CONSTANT(DrawGrassBladesPush, push)

#if DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_VERTEX

layout(location = 1) out vec3 v_color;

void main() {
    Vertex vert = deref_i(push.vertex_buffer, gl_VertexIndex);
    GrassBlade blade = deref_i(push.blade_buffer, gl_InstanceIndex);

    vec3 bezierPos = bezier(vert.position.y, blade.c0, blade.c1, blade.c2);
    vec3 facingDirection = vec3(cos(blade.angle), 0.0, sin(blade.angle));
    vec3 tangent = normalize(cross(-facingDirection, UP));

    vec4 worldPos = vec4(bezierPos, 1);
    worldPos.xyz += vert.position.z * tangent * 0.7; // PARAM: width

    gl_Position = deref(push.attachments.camera).proj_view * worldPos;
    // v_color = random_color(push.chunk_seed);
    // v_color = blade.color;
    v_color = mix(vec3(0.13, 0.26, 0.06), vec3(0.41, 0.61, 0.22), vert.position.y);
}

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_FRAGMENT

layout(location = 1) in vec3 v_color;
layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(v_color, 1.0);
}

#endif
