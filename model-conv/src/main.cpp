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

#include "../../include/transform.h"

mat4 ConvertMatrix(aiMatrix4x4 &mat)
{
    aiVector3D pos;
    aiQuaternion rot;
    aiVector3D scale;
    mat.Decompose(scale, rot, pos);

    vec3 p(pos.x, pos.y, pos.z);
    quat r(rot.x, rot.y, rot.z, rot.w);
    vec3 s(scale.x, scale.y, scale.z);

    return Translate(p) * mat4(r) * Scale(s);
}

typedef std::runtime_error Exception;

std::map<std::string, unsigned int> nodemaps;
unsigned int GetNode(const std::string &name)
{
    if (nodemaps.find(name) != nodemaps.end())
        return nodemaps[name];
    for (auto it = nodemaps.begin(); it != nodemaps.end(); ++it)
    {
        unsigned int fnd = it->first.find(name);
        if (fnd < it->first.length())
            return it->second;
    }
    return 0;   // if nothing, affected by root
}

std::fstream file;
unsigned int nodesNum = 0;
void WriteNode(aiNode* node)
{
    nodemaps[node->mName.C_Str()] = nodesNum;
    file.write((char*)&nodesNum, sizeof(nodesNum));
    file.write((char*)&node->mNumMeshes, sizeof(unsigned int));
    file.write((char*)node->mMeshes, sizeof(unsigned int)*node->mNumMeshes);
    file.write((char*)&node->mNumChildren, sizeof(unsigned int));
    nodesNum++;
    for (unsigned int i=0; i<node->mNumChildren; ++i)
        WriteNode(node->mChildren[i]);
}

int main()
{
    try
    {
        Assimp::Importer importer;
        const std::string iFilename = "StickmanAnim.dae";
        const std::string oFilename = "test.dat";

        const aiScene* scene = importer.ReadFile(iFilename.c_str(),
                        aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                        aiProcess_SortByPType | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);

        if (!scene)
            throw Exception(importer.GetErrorString());
   
        file.open(oFilename, std::ios::binary | std::ios::out);
        if (!file.good())
            throw Exception("Couldn't open file: " + oFilename);

        if (scene->mNumMeshes == 0)
            throw Exception("No meshes in the file");

        bool animated = false;
        if (scene->mNumAnimations > 0)
        {
            std::cout << "Want to save animation? (y/n): ";
            char c;
            std::cin >> c;
            if (c == 'y')
                animated = true;
        }

        if (animated)
            WriteNode(scene->mRootNode);

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

        if (animated)
        {
            file.write((char*)&mesh->mNumBones, sizeof(mesh->mNumBones));
            for (size_t j=0; j<mesh->mNumBones; ++j)
            {
                aiBone * bone = mesh->mBones[j];
                std::string name(bone->mName.C_Str());
                unsigned int id = GetNode(name);
                file.write((char*)&id, sizeof(id));
                
                mat4 m = ConvertMatrix(bone->mOffsetMatrix);
                file.write((char*)&m, sizeof(m));

                file.write((char*)&bone->mNumWeights, sizeof(bone->mNumWeights));
                file.write((char*)bone->mWeights, sizeof(aiVertexWeight)*bone->mNumWeights);
            }

            aiAnimation * anim = scene->mAnimations[0];
            file.write((char*)&anim->mDuration, sizeof(anim->mDuration));
            file.write((char*)&anim->mNumChannels, sizeof(anim->mNumChannels));
            for (size_t j=0; j<anim->mNumChannels; ++j)
            {
                aiNodeAnim* chn = anim->mChannels[j];
                std::string name(chn->mNodeName.C_Str());
                unsigned int id = GetNode(name);
                file.write((char*)&id, sizeof(id));
                
                file.write((char*)&chn->mNumPositionKeys, sizeof(chn->mNumPositionKeys));
                file.write((char*)&chn->mNumRotationKeys, sizeof(chn->mNumRotationKeys));
                file.write((char*)&chn->mNumScalingKeys, sizeof(chn->mNumScalingKeys));

                struct RotKey
                {
                    double time;
                    quat rot;
                };

                std::vector<RotKey> rotkeys(chn->mNumRotationKeys);
                for (size_t k=0; k<chn->mNumRotationKeys; ++k)
                {
                    rotkeys[k].time = chn->mRotationKeys[k].mTime;
                    rotkeys[k].rot.x = chn->mRotationKeys[k].mValue.x;
                    rotkeys[k].rot.y = chn->mRotationKeys[k].mValue.y;
                    rotkeys[k].rot.z = chn->mRotationKeys[k].mValue.z;
                    rotkeys[k].rot.w = chn->mRotationKeys[k].mValue.w;
                }

                file.write((char*)chn->mPositionKeys, sizeof(aiVectorKey)*chn->mNumPositionKeys);
                file.write((char*)&rotkeys[0], sizeof(RotKey)*chn->mNumRotationKeys);
                file.write((char*)chn->mScalingKeys, sizeof(aiVectorKey)*chn->mNumScalingKeys);
            }
        }

        file.close();
    }
    catch(std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
    }
    return 0;
}
