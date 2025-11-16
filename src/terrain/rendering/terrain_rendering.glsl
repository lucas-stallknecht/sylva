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

#define MIN_TESS_LEVEL 4
#define MAX_TESS_LEVEL 32
#define MIN_DISTANCE  5.0
#define MAX_DISTANCE  15.0

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tc_uv[gl_InvocationID] = v_uv[gl_InvocationID];

    // Invocation 0 is responsible for setting tessellation levels for the entire patch.
    if (gl_InvocationID == 0) {
        vec3 cam_pos = deref(push.camera_buffer).position.xyz;

        float d00 = distance(gl_in[0].gl_Position.xyz, cam_pos);
        float d01 = distance(gl_in[1].gl_Position.xyz, cam_pos);
        float d10 = distance(gl_in[2].gl_Position.xyz, cam_pos);
        float d11 = distance(gl_in[3].gl_Position.xyz, cam_pos);

        // Normalize distances 0..1
        float dist00 = clamp((d00 - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);
        float dist01 = clamp((d01 - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);
        float dist10 = clamp((d10 - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);
        float dist11 = clamp((d11 - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);

        float tess0 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist10, dist00));
        float tess1 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist00, dist01));
        float tess2 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist01, dist11));
        float tess3 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist11, dist10));

        gl_TessLevelOuter[0] = tess0;
        gl_TessLevelOuter[1] = tess1;
        gl_TessLevelOuter[2] = tess2;
        gl_TessLevelOuter[3] = tess3;

        gl_TessLevelInner[0] = max(tess1, tess3);
        gl_TessLevelInner[1] = max(tess0, tess2);
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
    gl_Position = deref(push.camera_buffer).proj_view * pos;
}

#elif DAXA_SHADER_STAGE == DAXA_SHADER_STAGE_FRAGMENT

layout(location = 0) in vec2 te_uv;
layout(location = 0) out vec4 out_color;

void main()
{
    daxa_ImageViewId albedo_img = push.attachments.terrain_albedo_map;
    daxa_ImageViewId normal_img = push.attachments.terrain_normal_map;
    daxa_SamplerId samp = push.linear_sampler;

    vec3 albedo = texture(daxa_sampler2D(albedo_img, samp), te_uv).rgb;
    albedo = pow(albedo, vec3(2.2));
    vec3 normal = texture(daxa_sampler2D(normal_img, samp), te_uv).rgb;
    normal = normal * 2.0 - 1.0;

    vec3 lightDir = normalize(vec3(0.4, 1.0, -0.2));

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = albedo * diff;
    vec3 ambient = albedo * 0.05;

    vec3 color = ambient + diffuse;

    out_color = vec4(pow(color, vec3(1.0 / 2.2)), 1);
}

#endif
