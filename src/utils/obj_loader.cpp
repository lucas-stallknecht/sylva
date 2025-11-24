#include "obj_loader.h"

#include <array>
#include <cstddef>
#include <fstream>
#include <optional>
#include <sstream>
#include <vector>
#include <memory>
#include <iostream>

namespace sylva
{

    std::optional<FaceVertexTriplet> parse_face_triplet(std::string const & token)
    {
        std::istringstream ss(token);
        char slash;
        FaceVertexTriplet out{};

        ss >> out.v >> slash >> out.vt >> slash >> out.vn;

        if (ss.fail())
        {
            return std::nullopt;
        }
        return out;
    }

    std::optional<std::shared_ptr<std::vector<Vertex>>>
    load_obj_vertices(std::string const & file_path)
    {
        std::ifstream file{file_path};

        if (!file.is_open())
        {
            std::cerr << "Failed to open asset file " << file_path << "\n";
            return std::nullopt;
        }

        std::vector<glm::vec3> temp_positions;
        std::vector<glm::vec2> temp_uvs;
        std::vector<glm::vec3> temp_normals;

        // We construct Vertex objects as we parse faces
        std::vector<Vertex> vertices;

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty())
                continue;

            std::istringstream ss{line};
            std::string prefix;
            ss >> prefix;

            if (prefix == "v")
            {
                glm::vec3 pos{};
                ss >> pos.x >> pos.y >> pos.z;

                if (ss.fail())
                {
                    std::cerr << "(" << file_path << ") Invalid vertex line: " << line << "\n";
                    return std::nullopt;
                }
                temp_positions.push_back(pos);
            }
            else if (prefix == "vn")
            {
                glm::vec3 normal{};
                ss >> normal.x >> normal.y >> normal.z;

                if (ss.fail())
                {
                    std::cerr << "(" << file_path << ") Invalid normal line: " << line << "\n";
                    return std::nullopt;
                }
                temp_normals.push_back(normal);
            }
            else if (prefix == "vt")
            {
                glm::vec2 uv{};
                ss >> uv.x >> uv.y;

                if (ss.fail())
                {
                    std::cerr << "(" << file_path << ") Invalid texture coordinate line: " << line
                              << "\n";
                    return std::nullopt;
                }
                temp_uvs.push_back(uv);
            }
            else if (prefix == "f")
            {
                std::array<std::string, 3> tokens{};
                ss >> tokens[0] >> tokens[1] >> tokens[2];

                if (ss.fail())
                {
                    std::cerr << "(" << file_path << ") Invalid face line: " << line << "\n";
                    return std::nullopt;
                }

                for (auto const & token : tokens)
                {
                    auto const vertex_triplet = parse_face_triplet(token);

                    if (!vertex_triplet)
                    {
                        std::cerr << "(" << file_path << ") Invalid face format: " << token << "\n";
                        return std::nullopt;
                    }

                    if (vertex_triplet->v == 0 || vertex_triplet->v > temp_positions.size() ||
                        vertex_triplet->vt == 0 || vertex_triplet->vt > temp_uvs.size() ||
                        vertex_triplet->vn == 0 || vertex_triplet->vn > temp_normals.size())
                    {
                        std::cerr << "(" << file_path
                                  << ") Invalid index reference in face: " << token << "\n";
                        return std::nullopt;
                    }

                    auto const pos_idx = static_cast<std::size_t>(vertex_triplet->v - 1);
                    auto const uv_idx = static_cast<std::size_t>(vertex_triplet->vt - 1);
                    auto const norm_idx = static_cast<std::size_t>(vertex_triplet->vn - 1);

                    vertices.push_back(Vertex{
                        .position = std::bit_cast<daxa_f32vec3>(temp_positions[pos_idx]),
                        .uv_1 = temp_uvs[uv_idx].x,
                        .normal = std::bit_cast<daxa_f32vec3>(temp_normals[norm_idx]),
                        .uv_2 = temp_uvs[uv_idx].y,
                    });
                }
            }
            // other prefixes (e.g. "o", "s", "g", "mtllib", "usemtl") are ignored for now
        }

        file.close();

        if (vertices.empty())
        {
            std::cerr << "(" << file_path << ") No vertex data produced from OBJ\n";
            return std::nullopt;
        }

        auto verts_ptr = std::make_shared<std::vector<Vertex>>(std::move(vertices));
        return verts_ptr;
    }

} // namespace sylva
