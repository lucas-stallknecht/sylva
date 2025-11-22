#pragma once

#include <daxa/daxa.hpp>
#include <daxa/utils/task_graph.hpp>

namespace sylva
{

    void upload_buffer(daxa::TaskInterface & task_interface, daxa::BufferId buffer_id,
                       void const * data, daxa::usize size);
    daxa::TaskBuffer create_and_upload_buffer(daxa::Device & device, daxa::BufferId buffer_id,
                                              void const * data, daxa::usize size,
                                              std::string name);
    daxa::TaskBuffer create_and_upload_buffer(daxa::Device & device, void const * data,
                                              daxa::BufferInfo const & buffer_info);
} // namespace sylva
