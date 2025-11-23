#pragma once

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>

namespace sylva
{
    struct UploadedBufferResult
    {
        daxa::BufferId buffer_id;
        daxa::TaskBuffer task_buffer;
    };

    void upload_buffer(daxa::TaskInterface & task_interface, daxa::BufferId buffer_id,
                       void const * data, daxa::usize size);
    UploadedBufferResult create_and_upload_buffer(daxa::Device & device, daxa::BufferId buffer_id,
                                                  void const * data, daxa::usize size,
                                                  std::string name);
    UploadedBufferResult create_and_upload_buffer(daxa::Device & device, void const * data,
                                                  daxa::BufferInfo const & buffer_info);
} // namespace sylva
