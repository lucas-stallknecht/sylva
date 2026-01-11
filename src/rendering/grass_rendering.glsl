#include <daxa/daxa.inl>

#define UP vec3(0.0, 1.0, 0.0)

#extension GL_EXT_debug_printf : enable

#include "opaque.inl"
#include "../shader_shared/random.glsl"
#include "../shader_shared/bezier.glsl"

DAXA_DECL_PUSH_CONSTANT(DrawGrassBladesPush, push)

// PARAM:
#define K 36.0
#define BLADE_WIDTH 0.55
#define FULLNESS_SCALE 0.3

#if DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_VERTEX

layout(location = 1) out vec3 v_color;

float displace_view_space_mag(float n_dot_v, float k) {
    return sign(n_dot_v) * exp(-k * n_dot_v * n_dot_v);
}

void main() {
    Vertex vert = deref_i(push.vertex_buffer, gl_VertexIndex);
    GrassBlade blade = deref_i(push.blade_buffer, gl_InstanceIndex);
    CamInfo camera = deref(push.attachments.camera);

    vec3 bezier_pos = bezier(vert.position.y, blade.c0, blade.c1, blade.c2);
    vec3 blade_normal = vec3(cos(blade.angle), 0.0, sin(blade.angle));
    vec3 tangent = normalize(cross(-blade_normal, UP));

    vec4 world_pos = vec4(bezier_pos, 1);
    world_pos.xyz += vert.position.z * tangent * BLADE_WIDTH;

    vec3 view_dir = normalize(camera.position.xyz - world_pos.xyz);
    float n_dot_v = dot(blade_normal, view_dir);

    vec4 view_pos = camera.view * world_pos;
    vec3 view_tangent = normalize((camera.view * vec4(tangent, 0.0)).xyz);
    // TODO(lstallknecht): replace fixed vector by normal or other direction
    view_pos.xyz += vec3(1.0, 0.0, 0.0) * displace_view_space_mag(n_dot_v, 36.0) * vert.position.z * FULLNESS_SCALE * BLADE_WIDTH;

    vec4 clip_pos = camera.proj * view_pos;

    gl_Position = clip_pos;
    // v_color = random_color(push.chunk_seed);
    // v_color = blade.color;
    v_color = mix(vec3(0.08, 0.16, 0.03), vec3(0.21, 0.51, 0.12), vert.position.y);
}

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_FRAGMENT

layout(location = 1) in vec3 v_color;
layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(v_color, 1.0);
}

#endif
