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

struct Vec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;
};

/// @brief Mesh in a scene.
struct Mesh
{

    /// @brief Default constructor with an optional name.
    Mesh(const std::string& name = "") : Name(name) {}

    /// @brief Name of the mesh.
    std::string Name = "";

    /// @brief Positions of the vertices.
    std::vector<Vec3> Positions = {};

    /// @brief Normals of the vertices.
    std::vector<Vec3> Normals = {};

    /// @brief Texture coordinates of the vertices.
    std::vector<Vec2> TexCoords = {};

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

/// @brief Wavefront OBJ file loader. Only supports triangulated meshes and doesn't support negative indices.
class WavefrontLoader
{
private:
    enum class FaceType
    {
        None,
        Vertex,
        VertexNormal,
        VertexTextureNormal,
        VertexTexture
    };

public:
    WavefrontLoader() = default;
    ~WavefrontLoader() = default;

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

        FaceType faceType = FaceType::None;

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

            if (outScene.Meshes.empty() || line[0] == 'o')
            {
                outScene.Meshes.push_back(Mesh(line.substr(2)));
            }

            Mesh& mesh = outScene.Meshes.back();

            if (line[0] == 'v')
            {
                if (line[1] == ' ')
                {
                    Vec3 position = mesh.Positions.emplace_back();

                    int n = std::sscanf(line.c_str(), "v %f %f %f", &position.x, &position.y, &position.z);

                    assert(n == 3);
                }
                else if (line[1] == 'n')
                {
                    Vec3& normal = mesh.Normals.emplace_back();

                    int n = std::sscanf(line.c_str(), "vn %f %f %f", &normal.x, &normal.y, &normal.z);

                    assert(n == 3);
                }
                else if (line[1] == 't')
                {
                    Vec2& texCoord = mesh.TexCoords.emplace_back();

                    int n = std::sscanf(line.c_str(), "vt %f %f", &texCoord.x, &texCoord.y);

                    assert(n == 2);
                }
            }
            else if (line[0] == 'f')
            {
                if (faceType == FaceType::None)
                {
                    faceType = DetermineFaceType(line);
                }

                mesh.Indices.emplace_back(ParseIndex(line, faceType));
            }
        }
    }

    /// @brief Convert a mesh to a continous array of vertices so that it can be used with graphics APIs.
    /// The layout of the vertices is as follows:
    /// - Position (3 floats)
    /// - Normal (3 floats) if the mesh has normals
    /// - Texture coordinate (2 floats) if the mesh has texture coordinates
    /// - Additional data (N floats) if you reserved N floats for each vertex
    /// Total stride is 3 + (hasNormals ? 3 : 0) + (hasTexCoords ? 2 : 0) + reserve_floats.
    /// @param mesh Mesh to convert.
    /// @param reserve_floats Number of floats to reserve for each vertex so that you can add additional data to each
    /// vertex. For example, if you want to add a color (3 floats) to each vertex, you would pass 3 here.
    /// @return The vertices of the mesh.
    std::vector<float> ConvertToStrided(Mesh const& mesh, uint32_t reserve_floats = 0)
    {

        assert(mesh.Positions.size() % 3);
        assert(mesh.Positions.size() == mesh.Normals.size());
        assert(mesh.Indices.size() % 3 == 0);

        std::vector<float> vertices(mesh.Positions.size() + mesh.Normals.size() + mesh.TexCoords.size());

        const bool hasNormals = !mesh.Normals.empty();
        const bool hasTexCoords = !mesh.TexCoords.empty();

        const uint32_t stride = 3 + (hasNormals ? 3 : 0) + (hasTexCoords ? 2 : 0) + reserve_floats;

        for (Index const& index : mesh.Indices)
        {
            uint8_t* data = reinterpret_cast<uint8_t*>(&vertices[index.vIdx * stride]);

            memcpy(data, &mesh.Positions[index.vIdx], sizeof(Vec3));

            data += sizeof(Vec3);

            if (hasNormals)
            {
                memcpy(data, &mesh.Normals[index.vnIdx], sizeof(Vec3));

                data += sizeof(Vec3);
            }
            if (hasTexCoords)
            {
                memcpy(data, &mesh.TexCoords[index.vtIdx], sizeof(Vec2));
            }
        }
        return std::move(vertices);
    }

private:
    Index ParseIndex(const std::string& line, FaceType faceType)
    {
        Index outIndex;

        if (faceType == FaceType::Vertex)
        {
            int n = std::sscanf(line.c_str(), "f %d %d %d", &outIndex.vIdx, &outIndex.vIdx, &outIndex.vIdx);

            assert(n == 3);
        }
        else if (faceType == FaceType::VertexNormal)
        {
            int n = std::sscanf(line.c_str(), "f %d//%d %d//%d %d//%d", &outIndex.vIdx, &outIndex.vnIdx, &outIndex.vIdx,
                                &outIndex.vnIdx, &outIndex.vIdx, &outIndex.vnIdx);

            assert(n == 6);
        }
        else if (faceType == FaceType::VertexTextureNormal)
        {
            int n = std::sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &outIndex.vIdx, &outIndex.vtIdx,
                                &outIndex.vnIdx, &outIndex.vIdx, &outIndex.vtIdx, &outIndex.vnIdx, &outIndex.vIdx,
                                &outIndex.vtIdx, &outIndex.vnIdx);

            assert(n == 9);
        }
        else if (faceType == FaceType::VertexTexture)
        {
            int n = std::sscanf(line.c_str(), "f %d/%d %d/%d %d/%d", &outIndex.vIdx, &outIndex.vtIdx, &outIndex.vIdx,
                                &outIndex.vtIdx, &outIndex.vIdx, &outIndex.vtIdx);

            assert(n == 6);
        }
    }

    FaceType DetermineFaceType(const std::string& line)
    {

        bool doubleSlashes = line.find("//") != std::string::npos;
        bool slashes = line.find("/") != std::string::npos;

        // If there are no slashes, the face type is Vertex.
        // eg. f 1 2 3
        if (!doubleSlashes && !slashes)
        {
            return FaceType::Vertex;
        }
        // If there are double slashes, the face type is VertexNormal.
        // eg. f 1//1 2//2 3//3
        else if (doubleSlashes)
        {
            return FaceType::VertexNormal;
        }
        else if (slashes)
        {
            // If there are no double slashes, the face type is VertexTextureNormal.
            // eg. f 1/1/1 2/2/2 3/3/3
            if (!doubleSlashes)
            {
                return FaceType::VertexTextureNormal;
            }
            // There are no double slashes, but there are slashes, so the face type is VertexTexture.
            // eg. f 1/1 2/2 3/3
            else
            {
                return FaceType::VertexTexture;
            }
        }
    }
};
