#include <daxa/daxa.inl>

#extension GL_EXT_debug_printf : enable

#include "terrain_rendering.inl"

DAXA_DECL_PUSH_CONSTANT(RenderTerrainPush, push)

#if DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_VERTEX

layout(location = 0) out vec2 v_uv;

void main()
{
    TerrainVertex vert = deref_i(push.attachments.vertices, gl_VertexIndex);

    gl_Position = vec4(vert.position, 1);
    v_uv = vert.uv;
}

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_TESSELATION_CONTROL

layout(vertices = 4) out;

layout(location = 0) in vec2 v_uv[];
layout(location = 0) out vec2 tc_uv[];

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tc_uv[gl_InvocationID] = v_uv[gl_InvocationID];

    // Invocation 0 is responsible for setting tessellation levels for the entire patch.
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 16.0;
        gl_TessLevelOuter[1] = 16.0;
        gl_TessLevelOuter[2] = 16.0;
        gl_TessLevelOuter[3] = 16.0;

        gl_TessLevelInner[0] = 16.0;
        gl_TessLevelInner[1] = 16.0;
    }
}

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_TESSELATION_EVALUATION

layout(quads, equal_spacing, ccw) in;

layout(location = 0) in vec2 tc_uv[];
layout(location = 0) out vec2 te_uv;

void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec4 p0 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, u);
    vec4 p1 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, u);
    vec4 pos = mix(p0, p1, v);

    te_uv = mix(mix(tc_uv[0], tc_uv[1], u),
            mix(tc_uv[2], tc_uv[3], u), v);

    daxa_ImageViewId img = push.attachments.terrain_height_map;
    daxa_SamplerId samp = push.linear_sampler;

    float height = texture(daxa_sampler2D(img, samp), te_uv).r;
    pos.y = height;

    // Now project to clip space
    gl_Position = push.proj_view * pos;
}

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_FRAGMENT

layout(location = 0) in vec2 te_uv;
layout(location = 0) out vec4 out_color;

void main()
{
    daxa_ImageViewId img = push.attachments.terrain_albedo_map;
    daxa_SamplerId samp = push.linear_sampler;

    vec3 albedo = texture(daxa_sampler2D(img, samp), te_uv).rgb;
    albedo = pow(albedo, vec3(2.2));

    vec3 lightDir = normalize(vec3(-1.0, 1.0, 1.0));

    // float diff = max(dot(te_normal, lightDir), 0.0);
    float diff = 1.0;
    vec3 diffuse = albedo * diff;
    vec3 ambient = albedo * 0.05;

    vec3 color = ambient + diffuse;

    out_color = vec4(pow(color, vec3(1.0 / 2.2)), 1);
}

#endif
