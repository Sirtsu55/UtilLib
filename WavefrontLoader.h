#pragma once

#include < vector>
#include < string>
#include < cstdint>
#include < fstream>
#include < optional>

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

    /// @brief Normals of the vertices. Empty if the mesh doesn't have normals.
    std::vector<Vec3> Normals = {};

    /// @brief Texture coordinates of the vertices. Empty if the mesh doesn't have texture coordinates.
    std::vector<Vec2> TexCoords = {};

    /// @brief Position indices.
    std::vector<uint32_t> vIDX = {};

    /// @brief Normal indices.
    std::vector<uint32_t> vnIDX = {};

    /// @brief Texture coordinate indices.
    std::vector<uint32_t> vtIDX = {};
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
        VertexTexture,
        None
    };

public:
    WavefrontLoader() = default;
    ~WavefrontLoader() = default;

    /// @brief Loads a scene from a file.  Only supports triangulated meshes that have at most 3 indices per face.
    /// Does not support negative indices.
    /// @param path Path to the file.
    /// @return The scene if it was loaded successfully.
    static std::optional<Scene> Load(const char* path)
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
                outScene.Meshes.push_back(Mesh(line.substr(2)));
            }

            if (line[0] == 'v')
            {
                if (outScene.Meshes.empty())
                {
                    outScene.Meshes.push_back(Mesh());
                }

                Mesh& mesh = outScene.Meshes.back();

                if (line[1] == ' ')
                {
                    Vec3& position = mesh.Positions.emplace_back();

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
                // Make sure that a mesh has been created.
                assert(outScene.Meshes.size() != 0);

                Mesh& mesh = outScene.Meshes.back();

                if (faceType == FaceType::None)
                {
                    faceType = DetermineFaceType(line);
                }

                ParseIndices(line, mesh.vIDX, mesh.vnIDX, mesh.vtIDX, faceType);
            }
        }

        return outScene;
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
    /// @return The vertices of the mesh. The new vertices can be indexed with the position indices. The normals
    /// and texture coord indices are irrelevant to the new vertices.
    static std::vector<float> ConvertToInterleaved(Mesh const& mesh, uint32_t reserve_floats = 0)
    {
        // Check that the mesh has positions are valid.
        assert(mesh.Positions.size() % 3);

        // Check that the indices are valid.
        assert(mesh.vIDX.size() % 3 == 0);

        uint32_t vertexCount = mesh.vIDX.size();

        const bool hasNormals = !mesh.Normals.empty();
        const bool hasTexCoords = !mesh.TexCoords.empty();

        const uint32_t stride = 3 + (hasNormals ? 3 : 0) + (hasTexCoords ? 2 : 0) + reserve_floats;

        std::vector<float> vertices(vertexCount * stride);

        for (uint32_t i = 0; i < mesh.vIDX.size(); ++i)
        {
            uint32_t posIdx = mesh.vIDX[i] - 1; // OBJ indices start at 1.

            uint8_t* data = reinterpret_cast<uint8_t*>(&vertices[posIdx * stride]);

            memcpy(data, &mesh.Positions[posIdx], sizeof(Vec3));

            data += sizeof(Vec3);

            if (hasNormals)
            {
                uint32_t normalIdx = mesh.vnIDX[i] - 1; // OBJ indices start at 1.
                memcpy(data, &mesh.Normals[normalIdx], sizeof(Vec3));

                data += sizeof(Vec3);
            }
            if (hasTexCoords)
            {
                uint32_t texCoordIdx = mesh.vtIDX[i] - 1; // OBJ indices start at 1.
                memcpy(data, &mesh.TexCoords[texCoordIdx], sizeof(Vec2));
            }
        }
        return vertices;
    }

private:
    static void ParseIndices(const std::string& line, std::vector<uint32_t>& vIndices, std::vector<uint32_t>& vnIndices,
                             std::vector<uint32_t>& vtIndices, FaceType faceType)
    {
        switch (faceType)
        {
        case WavefrontLoader::FaceType::Vertex:
        {
            vIndices.resize(vIndices.size() + 3);
            uint32_t* vStart = vIndices.data() + vIndices.size() - 3;

            int32_t n = std::sscanf(line.c_str(), "f %i %i %i", vStart, vStart + 1, vStart + 2);
            assert(n == 3);
            break;
        }
        case WavefrontLoader::FaceType::VertexNormal:
        {
            vIndices.resize(vIndices.size() + 3);
            vnIndices.resize(vnIndices.size() + 3);

            uint32_t* vStart = &vIndices[vIndices.size() - 3];
            uint32_t* vnStart = &vnIndices[vnIndices.size() - 3];

            int32_t n = std::sscanf(line.c_str(), "f %i//%i %i//%i %i//%i", vStart, vnStart, vStart + 1, vnStart + 1,
                                    vStart + 2, vnStart + 2);
            assert(n == 6);
            break;
        }
        case WavefrontLoader::FaceType::VertexTextureNormal:
        {
            vIndices.resize(vIndices.size() + 3);
            vtIndices.resize(vtIndices.size() + 3);
            vnIndices.resize(vnIndices.size() + 3);

            uint32_t* vStart = &vIndices[vIndices.size() - 3];
            uint32_t* vtStart = &vtIndices[vtIndices.size() - 3];
            uint32_t* vnStart = &vnIndices[vnIndices.size() - 3];

            int32_t n = std::sscanf(line.c_str(), "f %i/%i/%i %i/%i/%i %i/%i/%i", vStart, vtStart, vnStart, vStart + 1,
                                    vtStart + 1, vnStart + 1, vStart + 2, vtStart + 2, vnStart + 2);
            assert(n == 9);
            break;
        }
        case WavefrontLoader::FaceType::VertexTexture:
        {
            vIndices.resize(vIndices.size() + 3);
            vtIndices.resize(vtIndices.size() + 3);

            uint32_t* vStart = &vIndices[vIndices.size() - 3];
            uint32_t* vtStart = &vtIndices[vtIndices.size() - 3];

            int32_t n = std::sscanf(line.c_str(), "f %i/%i %i/%i %i/%i", vStart, vtStart, vStart + 1, vtStart + 1,
                                    vStart + 2, vtStart + 2);
            assert(n == 6);
            break;
        }
        case WavefrontLoader::FaceType::None:
        {
            // This should never happen.
            assert(false);
            break;
        }
        }
    }

    static FaceType DetermineFaceType(const std::string& line)
    {
        bool insideSpace = false;
        bool consecutiveSlashes = false;
        uint32_t slashCount = 0;
        uint32_t lastSlashIndex = ~0; // set to max vaie so that its not equal to 0.
        uint32_t index = 0;

        for (char c : line)
        {
            if (c == ' ')
            {
                if (insideSpace) // if we encounter a space after a space, we are done.
                {
                    break;
                }
                insideSpace = true;
            }
            else if (c == '/')
            {
                if (lastSlashIndex == index - 1)
                {
                    consecutiveSlashes = true;
                    return FaceType::VertexNormal;
                }
                lastSlashIndex = index;
                slashCount++;
            }
            index++;
        }

        switch (slashCount)
        {
        case 0: return FaceType::Vertex;
        case 1: return FaceType::VertexTexture;
        case 2: return FaceType::VertexTextureNormal;
        default: return FaceType::None;
        }
    }
};
