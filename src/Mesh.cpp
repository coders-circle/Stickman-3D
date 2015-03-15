#include <common.h>
#include <Mesh.h>
#include <transform.h>

Mesh::Mesh() : m_animation(NULL) {}

Mesh::~Mesh()
{
    if (m_animation)
        delete m_animation;
}

void Mesh::ReadNode(std::fstream& file, Node* node)
{
    unsigned int id, nChildren;
    file.read((char*)&id, sizeof(id));
    file.read((char*)&nChildren, sizeof(nChildren));
    m_animation->map[id] = node;
    node->children.resize(nChildren);
    for (unsigned int i=0; i<nChildren; ++i)
        ReadNode(file, &node->children[i]);
}
void Mesh::LoadAnimatedFile(const std::string &filename)
{
    if (!m_animation)
        m_animation = new AnimationInfo();

    std::fstream file;
    file.open(filename, std::ios::binary | std::ios::in);
    if (!file.good())
    {
        std::cout << "Couldn't load mesh from file: " << filename << std::endl;
        return;
    }

    ReadNode(file, &m_animation->root);

    std::vector<size_t> wtcnt;

    uint32_t nvertices;
    file.read((char*)&nvertices, sizeof(nvertices));
    m_vertices.resize(nvertices);
    m_animation->skin.resize(nvertices);
    m_animation->tempVertices.resize(nvertices);
    wtcnt.resize(nvertices, 0);

    file.read((char*)&m_vertices[0], nvertices*sizeof(Vertex));

    uint32_t nindices;
    file.read((char*)&nindices, sizeof(nindices));
    m_indices.resize(nindices);
    file.read((char*)&m_indices[0], nindices*sizeof(uint16_t));


    uint32_t nBones;
    file.read((char*)&nBones, sizeof(nBones));
    m_animation->bones.resize(nBones);
    for (size_t j=0; j<nBones; ++j)
    {
        Bone& bn = m_animation->bones[j];
        unsigned int id;
        file.read((char*)&id, sizeof(id));
        bn.node = m_animation->map[id];
        file.read((char*)&bn.offset, sizeof(bn.offset));

        uint32_t nWeights;
        file.read((char*)&nWeights, sizeof(nWeights));
        
        struct VWeight
        {
            unsigned int vid;
            float wt;
        };
        std::vector<VWeight> weights(nWeights);
        file.read((char*)&weights[0], sizeof(VWeight)*nWeights);
        
        for (size_t k=0; k<nWeights; ++k)
        {
            size_t wtid = wtcnt[weights[k].vid]++;
            m_animation->skin[weights[k].vid].weights[wtid] = weights[k].wt;
            m_animation->skin[weights[k].vid].boneids[wtid] = j;
        }
    }
   
    Animation &anim = m_animation->animation;
    file.read((char*)&anim.duration, sizeof(anim.duration));
    uint32_t anum;
    file.read((char*)&anum, sizeof(anum));
    anim.data.resize(anum);
    for (size_t j=0; j<anum; ++j)
    {
        NodeAnim& nd = anim.data[j];
        unsigned int id;
        file.read((char*)&id, sizeof(id));
        nd.node = m_animation->map[id];
        unsigned int knum;
        file.read((char*)&knum, sizeof(knum));
        nd.posKeys.resize(knum);
        file.read((char*)&knum, sizeof(knum));
        nd.rotKeys.resize(knum);
        file.read((char*)&nd.posKeys[0], sizeof(VecKey)*nd.posKeys.size());
        file.read((char*)&nd.rotKeys[0], sizeof(RotKey)*nd.rotKeys.size());
    }

    file.close();
}

void Mesh::Animate(double time)
{
    for (size_t i=0; i<m_animation->animation.data.size(); ++i)
    {
        NodeAnim& nd = m_animation->animation.data[i];
        size_t pk = 0, rk = 0;

        for (size_t j=1; j<nd.posKeys.size(); ++j)
            if (nd.posKeys[j].time > time)
            {
                pk = j-1;
                break;
            }

        for (size_t j=1; j<nd.rotKeys.size(); ++j)
            if (nd.rotKeys[j].time > time)
            {
                rk = j-1;
                break;
            }

        size_t npk = (pk + 1);//%nd.posKeys.size();
        size_t nrk = (rk + 1);//%nd.rotKeys.size();

        double dp = nd.posKeys[npk].time - nd.posKeys[pk].time;
        double dr = nd.rotKeys[nrk].time - nd.rotKeys[rk].time;

        float pf = 0.0f, rf = 0.0f;
        if (dp != 0)
            pf = float((time-nd.posKeys[pk].time)/dp);
        if (dr != 0)
            rf = float((time-nd.rotKeys[rk].time)/dr);

        vec3 pos = nd.posKeys[pk].vec + (nd.posKeys[npk].vec - nd.posKeys[pk].vec)*pf;
        quat rot = nd.rotKeys[rk].rot + (nd.rotKeys[nrk].rot - nd.rotKeys[rk].rot)*rf;
        nd.node->transform = Translate(pos) * mat4(rot);
    }

    UpdateNode(m_animation->root);
}

void Mesh::UpdateNode(Node& node, Node* parent)
{
    if (parent)
        node.combined_transform = parent->combined_transform * node.transform;
    else
        node.combined_transform = node.transform;
    for (size_t i=0; i<node.children.size(); ++i)
        UpdateNode(node.children[i], &node);
}

void Mesh::LoadFile(const std::string &filename)
{
    std::fstream file;
    file.open(filename, std::ios::binary | std::ios::in);
    if (!file.good())
    {
        std::cout << "Couldn't load mesh from file: " << filename << std::endl;
        return;
    }

    uint32_t nvertices;
    file.read((char*)&nvertices, sizeof(nvertices));
    m_vertices.resize(nvertices);
    file.read((char*)&m_vertices[0], nvertices*sizeof(Vertex));

    uint32_t nindices;
    file.read((char*)&nindices, sizeof(nindices));
    m_indices.resize(nindices);
    file.read((char*)&m_indices[0], nindices*sizeof(uint16_t));

    file.close();
}

void Mesh::LoadBox(float x, float y, float z)
{
    m_vertices.resize(24);
    m_vertices = std::vector<Vertex>
    ({
        // FRONT
        { vec3( x,  y,  z), vec3( 0,  0,  1), vec2(1.0f, 0.0f) },
        { vec3(-x,  y,  z), vec3( 0,  0,  1), vec2(0.0f, 0.0f) },
        { vec3( x, -y,  z), vec3( 0,  0,  1), vec2(1.0f, 1.0f) },
        { vec3(-x, -y,  z), vec3( 0,  0,  1), vec2(0.0f, 1.0f) },
        // RIGHT
        { vec3( x,  y, -z), vec3( 1,  0,  0), vec2(1.0f, 0.0f) },
        { vec3( x,  y,  z), vec3( 1,  0,  0), vec2(0.0f, 0.0f) },
        { vec3( x, -y, -z), vec3( 1,  0,  0), vec2(1.0f, 1.0f) },
        { vec3( x, -y,  z), vec3( 1,  0,  0), vec2(0.0f, 1.0f) },
        // LEFT
        { vec3(-x,  y,  z), vec3(-1,  0,  0), vec2(1.0f, 0.0f) },
        { vec3(-x,  y, -z), vec3(-1,  0,  0), vec2(0.0f, 0.0f) },
        { vec3(-x, -y,  z), vec3(-1,  0,  0), vec2(1.0f, 1.0f) },
        { vec3(-x, -y, -z), vec3(-1,  0,  0), vec2(0.0f, 1.0f) },
        // TOP
        { vec3( x,  y, -z), vec3( 0,  1,  0), vec2(1.0f, 0.0f) },
        { vec3(-x,  y, -z), vec3( 0,  1,  0), vec2(0.0f, 0.0f) },
        { vec3( x,  y,  z), vec3( 0,  1,  0), vec2(1.0f, 1.0f) },
        { vec3(-x,  y,  z), vec3( 0,  1,  0), vec2(0.0f, 1.0f) },
        // BOTTOM
        { vec3( x, -y,  z), vec3( 0, -1,  0), vec2(1.0f, 0.0f) },
        { vec3(-x, -y,  z), vec3( 0, -1,  0), vec2(0.0f, 0.0f) },
        { vec3( x, -y, -z), vec3( 0, -1,  0), vec2(1.0f, 1.0f) },
        { vec3(-x, -y, -z), vec3( 0, -1,  0), vec2(0.0f, 1.0f)},
        // BACK
        { vec3(-x,  y, -z), vec3( 0,  0, -1), vec2(1.0f, 0.0f) },
        { vec3( x,  y, -z), vec3( 0,  0, -1), vec2(0.0f, 0.0f) },
        { vec3(-x, -y, -z), vec3( 0,  0, -1), vec2(1.0f, 1.0f) },
        { vec3( x, -y, -z), vec3( 0,  0, -1), vec2(0.0f, 1.0f) },
    });
    
    m_indices.resize(36);
    m_indices = std::vector<uint16_t>
    ({
        0, 1, 3, 0, 3, 2,
        4, 5, 7, 4, 7, 6,
        8, 9, 11, 8, 11, 10,
        12, 13, 15, 12, 15, 14,
        16, 17, 19, 16, 19, 18,
        20, 21, 23, 20, 23, 22
    });
}

void Mesh::LoadSphere(float radius, uint16_t rings, uint16_t sectors)
{
    float R = 1.0f / float(rings-1);
    float S = 1.0f / float(sectors-1);
    uint16_t r, s;

    m_vertices.resize(rings*sectors);

    int i=0;
    for (r=0; r<rings; ++r)
        for (s=0; s<sectors; ++s)
        {
#define PI (3.14159265359f)
#define PI_2 (1.57079632679f)
            float y = -sinf(-PI_2 + PI * r * R);
            float x = cosf(2 * PI * s * S) * sinf(PI * r * R);
            float z = sinf(2 * PI * s * S) * sinf(PI * r * R);

            m_vertices[i].texcoords.x = s*S;
            m_vertices[i].texcoords.y = r*R;

            m_vertices[i].position.x = x*radius;
            m_vertices[i].position.y = y*radius;
            m_vertices[i].position.z = z*radius;
            
            m_vertices[i].normal.x = x;
            m_vertices[i].normal.y = y;
            m_vertices[i].normal.z = z;
            ++i;
        }

    m_indices.resize((rings-1)*(sectors-1)*6);
    auto id = &m_indices[0];
    for (r=0; r<rings-1; ++r)
        for (s=0; s<sectors-1; ++s)
        {
            *id++ = uint16_t(r*sectors + s);
            *id++ = uint16_t(r*sectors + s+1);
            *id++ = uint16_t((r+1)*sectors + s+1);
            *id++ = uint16_t(r*sectors + s);
            *id++ = uint16_t((r+1)*sectors + s+1);
            *id++ = uint16_t((r+1)*sectors + s);
        }
    
}

// Instantiation with each material class to avoid linking issues
//template class Mesh<TextureShadowMaterial>;
