#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <cassert>
#include <stdint.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>

typedef std::runtime_error Exception;

int main()
{
    try
    {
        Assimp::Importer importer;
        const std::string iFilename = "Stickman.3ds";
        const std::string oFilename = "test.dat";

        const aiScene* scene = importer.ReadFile(iFilename.c_str(),
                        aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                        aiProcess_SortByPType | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);

        if (!scene)
            throw Exception(importer.GetErrorString());
   
        std::fstream file;
        file.open(oFilename, std::ios::binary | std::ios::out);
        if (!file.good())
            throw Exception("Couldn't open file: " + oFilename);

        if (scene->mNumMeshes == 0)
            throw Exception("No meshes in the file");

        // Import the mesh data and save it in the file
        aiMesh* mesh = scene->mMeshes[0];
        uint32_t nvertices;

        nvertices = mesh->mNumVertices;
        file.write((char*)&nvertices, sizeof(nvertices));

        struct Vertex
        {
            aiVector3D position, normal;
            aiVector2D tcoords;
        };
        std::vector<Vertex> vertices(nvertices);
        for (size_t i=0; i<mesh->mNumVertices; ++i)
        {
            vertices[i].position = mesh->mVertices[i];
            vertices[i].normal = mesh->mNormals[i];
            vertices[i].tcoords.x = (!mesh->mTextureCoords[0])?0.0f:mesh->mTextureCoords[0][i].x;
            vertices[i].tcoords.y = (!mesh->mTextureCoords[0])?0.0f:mesh->mTextureCoords[0][i].y;
        }
        file.write((char*)&vertices[0], sizeof(Vertex)*nvertices);

        unsigned int tris = mesh->mNumFaces*3;
        file.write((char*)&tris ,sizeof(tris));

        std::vector<uint16_t> indices(tris);
        for (size_t i=0; i<mesh->mNumFaces; ++i)
        {    
            indices[i*3 + 0] = mesh->mFaces[i].mIndices[0];
            indices[i*3 + 1] = mesh->mFaces[i].mIndices[1];
            indices[i*3 + 2] = mesh->mFaces[i].mIndices[2];
        }
        file.write((char*)&indices[0], sizeof(uint16_t)*tris);

        file.close();
    }
    catch(std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
    }
    return 0;
}
