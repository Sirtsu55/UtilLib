#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <cstdint>
#include <fstream>
#include <optional>

/// @brief Index of a vertex, normal, and texture coordinate.
struct Index
{
    /// @brief Index of the vertex.
    uint32_t vIdx = 0;

    /// @brief Index of the normal.
    uint32_t vnIdx = 0;

    /// @brief Index of the texture coordinate.
    uint32_t vtIdx = 0;
};

/// @brief Mesh in a scene.
struct Mesh
{

    /// @brief Default constructor with an optional name.
    Mesh(const std::string& name = "") : Name(name) {}

    /// @brief Name of the mesh.
    std::string Name = "";

    /// @brief Positions of the vertices.
    std::vector<float> Positions = {};

    /// @brief Normals of the vertices.
    std::vector<float> Normals = {};

    /// @brief Texture coordinates of the vertices.
    std::vector<float> TexCoords = {};

    /// @brief Indices of the vertices.
    std::vector<Index> Indices = {};
};

/// @brief Describes a scene.
struct Scene
{
    /// @brief Meshes in the scene.
    std::vector<Mesh> Meshes = {};

    /// @brief Materials in the scene.
    // std::vector<Material> Materials = {};
};

/// @brief Wavefront OBJ file loader.
class WavefrontLoader
{
private:
    enum class FaceType
    {
        Vertex,
        VertexNormal,
        VertexTextureNormal,
        VertexTexture
    };

public:
    WavefrontLoader() = default;
    ~WavefrontLoader();

    /// @brief Loads a scene from a file.
    /// @param path Path to the file.
    /// @return The scene if it was loaded successfully.
    std::optional<Scene> Load(std::filesystem::path const& path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            return std::nullopt;
        }

        Scene outScene;

        bool slashesCheck = false;
        FaceType faceType = FaceType::Vertex;

        for (std::string line; std::getline(file, line);)
        {
            if (line.empty() || line[0] == '#')
            {
                continue;
            }

            if (line[0] == 'o')
            {
                // Object
            }
            else if (line[0] == 'g')
            {
                // Group
            }

            if (outScene.Meshes.empty() || line[0] == 'o' || line[0] == 'g')
            {
                outScene.Meshes.push_back(Mesh());
            }

            Mesh& mesh = outScene.Meshes.back();

            if (line[0] == 'v')
            {
                if (line[1] == ' ')
                {

                    int n = std::sscanf(line.c_str(), "v %f %f %f", &mesh.Positions.emplace_back(),
                                        &mesh.Positions.emplace_back(), &mesh.Positions.emplace_back());

                    assert(n == 3);
                }
                else if (line[1] == 'n')
                {
                    int n = std::sscanf(line.c_str(), "vn %f %f %f", &mesh.Normals.emplace_back(),
                                        &mesh.Normals.emplace_back(), &mesh.Normals.emplace_back());

                    assert(n == 3);
                }
                else if (line[1] == 't')
                {
                    int n = std::sscanf(line.c_str(), "vt %f %f", &mesh.TexCoords.emplace_back(),
                                        &mesh.TexCoords.emplace_back());

                    assert(n == 2);
                }
            }
            else if (line[0] == 'f')
            {
                if (!slashesCheck)
                {
                    if (std::ranges::count(line, '/') == 0)
                    {
                        faceType = FaceType::Vertex;
                    }
                    else if (std::ranges::count(line, '/') == 3)
                    {
                        faceType = FaceType::VertexTextureNormal;
                    }
                    else if (std::ranges::count(line, '/') == 6)
                    {
                        faceType = FaceType::VertexTexture;
                    }
                    else if (std::ranges::count(line, '/') == 9)
                    {
                        faceType = FaceType::VertexNormal;
                    }
                    else
                    {
                        assert(false);
                    }

                    slashesCheck = true;
                }
            }
        }
    }
};
