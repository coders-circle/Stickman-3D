#pragma once

struct Node
{
    std::vector<Node> children;
    mat4 transform;
    mat4 combined_transform;
};

struct VecKey
{
    double time;
    vec3 vec;
};

struct RotKey
{
    double time;
    quat rot;
};

struct NodeAnim
{
    Node* node;
    std::vector<VecKey> posKeys;
    std::vector<RotKey> rotKeys;
};

struct Animation
{
    double duration;
    std::vector<NodeAnim> data;
    double speed;
};

struct Bone
{
    Node * node;
    mat4 offset;
};

struct WeightInfo
{
    WeightInfo()
    {
        for (int i=0; i<4; ++i)
        {
            boneids[i] = 0;
            weights[i] = 0;
        }
    }
    size_t boneids[4];
    float weights[4];
};
