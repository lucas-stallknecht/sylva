#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <vector>
#include <cstring>

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>
#include <daxa/utils/task_graph_types.hpp>

#include "gpu_context.h"
#include "window.h"
#include "tasks/tesselate_terrain.h"
#include "utils/upload_image.h"

constexpr std::size_t N_PATCHES = 20u;
constexpr float WIDTH_X = 1.0f;
constexpr float WIDTH_Y = 1.0f;
constexpr float INV_RES = 1.0f / static_cast<float>(N_PATCHES);

static std::shared_ptr<daxa::RasterPipeline> create_pipeline(sylva::GPUContext & context)
{
    auto res = context.pipeline_manager.add_raster_pipeline2(
        sylva::tesselate_terrain_pipeline_compile_info(context.swapchain.get_format()));
    if (res.is_err())
    {
        std::cerr << "Failed to create raster pipeline: " << res.message() << std::endl;
        return nullptr;
    }
    return res.value();
}

static std::optional<daxa::TaskImage> upload_height_map(sylva::GPUContext & context,
                                                        std::string_view path)
{
    auto res = sylva::upload_image(context.device, std::string(path),
                                   daxa::ImageInfo{
                                       .format = daxa::Format::R8_UNORM,
                                       .usage = daxa::ImageUsageFlagBits::TRANSFER_DST |
                                                daxa::ImageUsageFlagBits::SHADER_STORAGE |
                                                daxa::ImageUsageFlagBits::SHADER_SAMPLED,
                                       .name = "height_map_image",
                                   });
    if (!res.has_value())
    {
        std::cerr << "Failed to upload height map image: " << path << std::endl;
        return std::nullopt;
    }
    return res.value();
}

static std::vector<TerrainVertex> generate_terrain_vertices()
{
    std::vector<TerrainVertex> vertices;
    vertices.reserve(N_PATCHES * N_PATCHES * 4);

    for (std::size_t i = 0; i < N_PATCHES; ++i)
    {
        for (std::size_t j = 0; j < N_PATCHES; ++j)
        {
            float const fi = static_cast<float>(i);
            float const fj = static_cast<float>(j);
            float const fi1 = fi + 1.0f;
            float const fj1 = fj + 1.0f;

            vertices.push_back(TerrainVertex{
                .position = {-WIDTH_X * 0.5f + WIDTH_X * fi * INV_RES, 0.0f,
                             -WIDTH_Y * 0.5f + WIDTH_Y * fj * INV_RES},
                .uv = {fi * INV_RES, fj * INV_RES},
            });

            vertices.push_back(TerrainVertex{
                .position = {-WIDTH_X * 0.5f + WIDTH_X * fi1 * INV_RES, 0.0f,
                             -WIDTH_Y * 0.5f + WIDTH_Y * fj * INV_RES},
                .uv = {fi1 * INV_RES, fj * INV_RES},
            });

            vertices.push_back(TerrainVertex{
                .position = {-WIDTH_X * 0.5f + WIDTH_X * fi * INV_RES, 0.0f,
                             -WIDTH_Y * 0.5f + WIDTH_Y * fj1 * INV_RES},
                .uv = {fi * INV_RES, fj1 * INV_RES},
            });

            vertices.push_back(TerrainVertex{
                .position = {-WIDTH_X * 0.5f + WIDTH_X * fi1 * INV_RES, 0.0f,
                             -WIDTH_Y * 0.5f + WIDTH_Y * fj1 * INV_RES},
                .uv = {fi1 * INV_RES, fj1 * INV_RES},
            });
        }
    }

    return vertices;
}

static daxa::BufferId create_and_upload_vertex_buffer(daxa::Device & device,
                                                      std::span<TerrainVertex const> vertices)
{
    auto const buffer_size = sizeof(TerrainVertex) * vertices.size();

    auto buffer_id = device.create_buffer({
        .size = buffer_size,
        .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
        .name = "terrain_vertex_buffer",
    });

    // Map and copy vertex data
    auto ptr_opt = device.buffer_host_address_as<TerrainVertex>(buffer_id);
    if (!ptr_opt.has_value())
    {
        std::cerr << "Failed to map vertex buffer for write." << std::endl;
        device.destroy_buffer(buffer_id);
        return {};
    }
    TerrainVertex * mapped = ptr_opt.value();
    std::memcpy(mapped, vertices.data(), buffer_size);

    return buffer_id;
}

int main()
{
    sylva::Window window("Sylva", 800, 600);
    sylva::GPUContext context(window);

    // Pipeline
    auto pipeline = create_pipeline(context);
    if (!pipeline)
        return 1;

    // Height map (task image)
    auto task_height_map_opt = upload_height_map(context, "resources/textures/terrain_height.png");
    if (!task_height_map_opt.has_value())
        return 1;
    daxa::TaskImage task_height_map = task_height_map_opt.value();

    // Terrain vertices
    auto const vertices = generate_terrain_vertices();
    auto const vertex_buffer_id =
        create_and_upload_vertex_buffer(context.device, std::span{vertices});
    if (vertex_buffer_id.is_empty())
    {
        std::cerr << "Failed to create vertex buffer." << std::endl;
        return 1;
    }

    auto task_vertex_buffer = daxa::TaskBuffer({
        .initial_buffers = {.buffers = std::span{&vertex_buffer_id, 1}},
        .name = "task_terrain_vertex_buffer",
    });

    // Task graph
    auto tg = daxa::TaskGraph({
        .device = context.device,
        .swapchain = context.swapchain,
        .name = "loop_task_graph",
    });

    auto task_swapchain_image = daxa::TaskImage({
        .swapchain_image = true,
        .name = "task_sc_image",
    });

    tg.use_persistent_buffer(task_vertex_buffer);
    tg.use_persistent_image(task_height_map);
    tg.use_persistent_image(task_swapchain_image);

    tg.add_task(sylva::TesselateTerrainTask{
        .views =
            {
                .vertices = task_vertex_buffer.view(),
                .height_map = task_height_map.view(),
                .dst_img = task_swapchain_image.view(),
            },
        .pipeline = pipeline.get(),
        .vertex_count = static_cast<daxa::u32>(vertices.size()),
    });

    tg.submit({});
    tg.present({});
    tg.complete({});

    // Main loop
    while (!window.should_close())
    {
        window.update();

        if (window.swapchain_out_of_date)
        {
            context.swapchain.resize();
            window.swapchain_out_of_date = false;
        }

        auto swapchain_image = context.swapchain.acquire_next_image();
        if (swapchain_image.is_empty())
            continue;

        task_swapchain_image.set_images({.images = std::span{&swapchain_image, 1}});

        tg.execute({});
        context.device.collect_garbage();
    }

    context.device.wait_idle();

    // Cleanup
    context.device.destroy_buffer(task_vertex_buffer.get_state().buffers[0]);
    context.device.destroy_image(task_height_map.get_state().images[0]);
    context.device.collect_garbage();

    return 0;
}
