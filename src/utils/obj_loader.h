#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>

#include "../shader_shared/shared.inl"

namespace sylva
{

    struct FaceVertexTriplet
    {
        uint32_t v, vt, vn;
    };

    std::optional<FaceVertexTriplet> parse_face_triplet(std::string const & token);
    std::optional<std::shared_ptr<std::vector<Vertex>>>
    load_obj_vertices(std::string const & file_path);

} // namespace sylva
