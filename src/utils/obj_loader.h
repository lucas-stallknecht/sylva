#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "../common.h"

namespace sylva
{

    struct FaceVertexTriplet
    {
        uint32_t v, vt, vn;
    };

    std::optional<FaceVertexTriplet> parse_face_triplet(std::string const & token);
    std::optional<std::shared_ptr<std::vector<Vertex>>>
    load_obj_mesh(std::string const & file_path);

} // namespace sylva
