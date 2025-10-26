#pragma once

#include <optional>
#include <stb/stb_image.h>
#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>

using namespace daxa::types;

namespace sylva
{

    // TODO(lstallknecht): Make upload TaskGraph reusable for multiple uploads...
    inline std::optional<daxa::TaskImage> upload_image(daxa::Device device, std::string image_path,
                                                       daxa::ImageInfo image_info)
    {
        stbi_uc * pixels_u8 = nullptr;
        float * pixels_f32 = nullptr;
        bool is_hdr = stbi_is_hdr(image_path.c_str());

        int tex_width{};
        int tex_height{};
        int channels = 0;

        u32 bytes_per_pixel = 0;

        if (is_hdr)
        {
            pixels_f32 = stbi_loadf(image_path.c_str(), &tex_width, &tex_height, &channels, 4);
            if (!pixels_f32)
                return std::nullopt;

            bytes_per_pixel = 4 * sizeof(float);                   // RGBA float
            image_info.format = daxa::Format::R32G32B32A32_SFLOAT; // float format
        }
        else
        {
            int stb_format = STBI_rgb_alpha;
            if (image_info.format == daxa::Format::R8_UNORM)
                stb_format = STBI_grey;

            pixels_u8 =
                stbi_load(image_path.c_str(), &tex_width, &tex_height, &channels, stb_format);
            if (!pixels_u8)
                return std::nullopt;

            bytes_per_pixel = (image_info.format == daxa::Format::R8_UNORM) ? 1u : 4u;
        }

        u32 tex_width_u = static_cast<u32>(tex_width);
        u32 tex_height_u = static_cast<u32>(tex_height);

        u32 image_size = tex_width_u * tex_height_u * bytes_per_pixel;

        image_info.size = {.x = tex_width_u, .y = tex_height_u, .z = 1};

        daxa::ImageId image_id = device.create_image(image_info);

        auto task_image = daxa::TaskImage({
            .initial_images = {.images = std::span{&image_id, 1}},
            .name = "task_" + std::string(image_info.name.data(), image_info.name.size()),
        });

        auto upload_tg = daxa::TaskGraph({
            .device = device,
            .name = "upload_image_task_graph",
        });

        upload_tg.use_persistent_image(task_image);
        upload_tg.add_task(
            daxa::Task::Transfer("upload_image_task")
                .writes(daxa::TaskStage::TRANSFER, task_image)
                .executes(
                    [&](daxa::TaskInterface ti)
                    {
                        daxa::BufferId staging_buffer_id = ti.device.create_buffer({
                            .size = image_size,
                            .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
                            .name = "staging_buffer",
                        });
                        ti.recorder.destroy_buffer_deferred(staging_buffer_id);

                        auto * mapped =
                            ti.device.buffer_host_address_as<std::byte>(staging_buffer_id).value();

                        if (is_hdr)
                        {
                            std::memcpy(mapped, pixels_f32, image_size);
                        }
                        else
                        {
                            std::memcpy(mapped, pixels_u8, image_size);
                        }

                        ti.recorder.copy_buffer_to_image({
                            .buffer = staging_buffer_id,
                            .image = image_id,
                            .image_extent =
                                {
                                    .x = tex_width_u,
                                    .y = tex_height_u,
                                    .z = 1,
                                },
                        });
                    }));
        upload_tg.submit({});
        upload_tg.complete({});
        upload_tg.execute({});

        if (is_hdr)
            stbi_image_free(pixels_f32);
        else
            stbi_image_free(pixels_u8);

        return task_image;
    }

} // namespace sylva
