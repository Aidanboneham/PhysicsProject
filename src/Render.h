#ifndef _RENDER_H_
#define _RENDER_H_

#define GLM_SWIZZLE
#include "glm/glm.hpp"
#include "FBXFile.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

struct Material
{
    int diffuse_texture;
    int normal_texture;
    int specular_texture;

    vec3 diffuse_color;
    vec3 specular_color;
};

struct SimpleVertex
{
    float pos[3];
    float normal[3];
    float binormal[3];
    float tangent[3];
    float uv[2];
};

struct Mesh
{
    unsigned int vbo;
    unsigned int ibo;
    unsigned int vao;
    int index_count;
    int vertex_count;

    SimpleVertex* vertex_data;
};

struct RenderItem
{
    Mesh* mesh;
    Material* material;
    mat4 transform;
};

Mesh BuildStaticMeshByMaterial(FBXFile * file, const char* material_name, bool store_vertex_data = false);
Material GetMaterial(FBXFile * file, const char* name);

class Renderer
{
public:
    Renderer();
    ~Renderer();
    void PushMesh(Mesh* mesh, Material* material, mat4 transform);
    void RenderAndClear(mat4 view_proj);

    RenderItem* render_queue;
    int queue_used;
    int queue_allocated;

    unsigned int main_shader;
};

#endif