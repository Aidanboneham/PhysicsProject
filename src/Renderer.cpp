#include "Render.h"

#include <vector>
#include "gl_core_4_4.h"
#include "stb_image.h"
#include "GLFW/glfw3.h"

bool LoadShaderType(char* filename,
    GLenum shader_type,
    unsigned int* output)
{
    //we want to be able to return if we succeded
    bool succeeded = true;

    //open the shader file
    FILE* shader_file = fopen(filename, "r");

    //did it open successfully 
    if (shader_file == 0)
    {
        succeeded = false;
    }
    else
    {
        //find out how long the file is
        fseek(shader_file, 0, SEEK_END);
        int shader_file_length = ftell(shader_file);
        fseek(shader_file, 0, SEEK_SET);
        //allocate enough space for the file
        char *shader_source = new char[shader_file_length];
        //read the file and update the length to be accurate
        shader_file_length = fread(shader_source, 1, shader_file_length, shader_file);

        //create the shader based on the type that got passed in
        unsigned int shader_handle = glCreateShader(shader_type);
        //compile the shader
        glShaderSource(shader_handle, 1, &shader_source, &shader_file_length);
        glCompileShader(shader_handle);

        //chech the shader for errors
        int success = GL_FALSE;
        glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE)
        {
            int log_length = 0;
            glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &log_length);
            char* log = new char[log_length];
            glGetShaderInfoLog(shader_handle, log_length, NULL, log);
            printf("%s\n", log);
            delete[] log;
            succeeded = false;
        }
        //only give the result to the caller if we succeeded
        if (succeeded)
        {
            *output = shader_handle;
        }

        //clean up the stuff we allocated
        delete[] shader_source;
        fclose(shader_file);
    }

    return succeeded;
}

bool LoadShader(
    char* vertex_filename,
    char* geometry_filename,
    char* fragment_filename,
    GLuint* result)
{
    bool succeeded = true;

    *result = glCreateProgram();

    unsigned int vertex_shader;

    if (LoadShaderType(vertex_filename, GL_VERTEX_SHADER, &vertex_shader))
    {
        glAttachShader(*result, vertex_shader);
        glDeleteShader(vertex_shader);
    }
    else
    {
        printf("FAILED TO LOAD VERTEX SHADER\n");
    }

    if (geometry_filename != nullptr)
    {
        unsigned int geometry_shader;
        if (LoadShaderType(geometry_filename, GL_GEOMETRY_SHADER, &geometry_shader))
        {
            glAttachShader(*result, geometry_shader);
            glDeleteShader(geometry_shader);
        }
        else
        {
            printf("FAILED TO LOAD GEOMETRY SHADER\n");
        }
    }
    if (fragment_filename != nullptr)
    {
        unsigned int fragment_shader;
        if (LoadShaderType(fragment_filename, GL_FRAGMENT_SHADER, &fragment_shader))
        {
            glAttachShader(*result, fragment_shader);
            glDeleteShader(fragment_shader);
        }
        else
        {
            printf("FAILED TO LOAD FRAGMENT SHADER\n");
        }
    }

    glLinkProgram(*result);

    GLint success;
    glGetProgramiv(*result, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        GLint log_length;
        glGetProgramiv(*result, GL_INFO_LOG_LENGTH, &log_length);
        char* log = new char[log_length];
        glGetProgramInfoLog(*result, log_length, 0, log);

        printf("ERROR: STUFF DONE SCREWED UP IN UR SHADER BUDDY!\n\n");
        printf("%s", log);

        delete[] log;
        succeeded = false;
    }

    return succeeded;
}

Mesh BuildStaticMeshByMaterial(FBXFile * file, const char* material_name, bool store_vertex_data)
{
    int mesh_count = (int)file->getMeshCount();
    int index_count = 0;
    int vertex_count = 0;

    FBXMaterial* mat = file->getMaterialByName(material_name);

    Mesh result = {};

    if (!mat) {
        return result;
    }

    for (int mesh_index = 0;
        mesh_index < mesh_count;
        ++mesh_index)
    {
        FBXMeshNode* mesh_node = file->getMeshByIndex(mesh_index);
        if (mesh_node->m_material == mat)
        {
            index_count += mesh_node->m_indices.size();
            vertex_count += mesh_node->m_vertices.size();
        }
    }

    unsigned int *index_data = new unsigned int[index_count];
    SimpleVertex *vertex_data = new SimpleVertex[vertex_count];

    unsigned int *index_cursor = index_data;
    SimpleVertex *vertex_cursor = vertex_data;
    int vertex_base = 0;

    for (int mesh_index = 0;
        mesh_index < mesh_count;
        ++mesh_index)
    {
        FBXMeshNode* mesh_node = file->getMeshByIndex(mesh_index);

        if (mesh_node->m_material == mat)
        {
            for (unsigned int i = 0; i < mesh_node->m_indices.size(); ++i)
            {
                *index_cursor++ = mesh_node->m_indices[i] + vertex_base;
            }

            for (unsigned int i = 0; i < mesh_node->m_vertices.size(); ++i)
            {
                vec3 pos = (mesh_node->m_globalTransform * mesh_node->m_vertices[i].position).xyz();
                vec3 normal = (mesh_node->m_globalTransform * mesh_node->m_vertices[i].normal).xyz();

                memcpy(vertex_cursor->pos, &pos, sizeof(float) * 3);
                memcpy(vertex_cursor->normal, &normal, sizeof(float) * 3);
                memcpy(vertex_cursor->binormal, &mesh_node->m_vertices[i].binormal, sizeof(float) * 3);
                memcpy(vertex_cursor->tangent, &mesh_node->m_vertices[i].tangent, sizeof(float) * 3);
                memcpy(vertex_cursor->uv, &mesh_node->m_vertices[i].texCoord1, sizeof(float) * 2);

                ++vertex_cursor;
            }

            vertex_base += mesh_node->m_vertices.size();
        }
    }

    result.index_count = index_count;
    result.vertex_count = vertex_count;

    glGenBuffers(1, &result.ibo);
    glGenBuffers(1, &result.vbo);
    glGenVertexArrays(1, &result.vao);

    glBindVertexArray(result.vao);
    glBindBuffer(GL_ARRAY_BUFFER, result.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result.ibo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(SimpleVertex) * vertex_count, vertex_data, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * index_count, index_data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)(sizeof(float) * 3));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)(sizeof(float) * 6));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)(sizeof(float) * 9));
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)(sizeof(float) * 12));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    delete[]index_data;

    if (store_vertex_data) {
        result.vertex_data = vertex_data;
    } else {
        delete[]vertex_data;
    }

    return result;
}

unsigned int
CreateGLTextureBasic(unsigned char* data, int width, int height, int channels)
{
    unsigned int tex_handle;
    glGenTextures(1, (GLuint*)&tex_handle);
    glBindTexture(GL_TEXTURE_2D, tex_handle);

    GLenum format = 0;
    GLenum src_format = 0;
    switch (channels)
    {
    case 1:
    {
        format = GL_RED;
        src_format = GL_R8;
    }break;
    case 2:
    {
        format = GL_RG;
        src_format = GL_RG8;
    }break;
    case 3:
    {
        format = GL_RGB;
        src_format = GL_RGB8;
    }break;
    case 4:
    {
        format = GL_RGBA;
        src_format = GL_RGBA8;
    }break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, src_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return tex_handle;
}

unsigned LoadGLTextureBasic(const char* filename)
{
    unsigned int result = 0;
    int width, height, channels;

    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (data)
    {
        result = CreateGLTextureBasic(data, width, height, channels);
        stbi_image_free(data);
    }

    return result;
}

Material GetMaterial(FBXFile * file, const char* name)
{
    Material result = {};
    
    FBXMaterial* mat = file->getMaterialByName(name);

    if (mat)
    {
        result.diffuse_color = mat->diffuse.rgb();
        result.specular_color= mat->specular.rgb();

        FBXTexture* diffuse_tex = mat->textures[FBXMaterial::TextureTypes::DiffuseTexture];
        FBXTexture* normals_tex = mat->textures[FBXMaterial::TextureTypes::NormalTexture];
        FBXTexture* specular_tex = mat->textures[FBXMaterial::TextureTypes::SpecularTexture];

        if (diffuse_tex) result.diffuse_texture = LoadGLTextureBasic(diffuse_tex->path.c_str());
        if (normals_tex) result.normal_texture = LoadGLTextureBasic(normals_tex->path.c_str());
        if (specular_tex) result.specular_texture = LoadGLTextureBasic(specular_tex->path.c_str());
    }

    return result;
}

Renderer::Renderer()
{
    LoadShader("./shaders/main_shader.vs", 0, "./shaders/main_shader.fs", &main_shader);
    queue_allocated = 4096;
    queue_used = 0;
    render_queue = new RenderItem[queue_allocated];
}

Renderer::~Renderer()
{
    delete[] render_queue;
}

void Renderer::PushMesh(Mesh* mesh, Material* material, mat4 transform)
{
    RenderItem item = {};
    item.mesh = mesh;
    item.material = material;
    item.transform = transform;

    if ( queue_used < queue_allocated )
    {
        render_queue[queue_used++] = item;
    }
}

void Renderer::RenderAndClear(mat4 view_proj)
{
    glUseProgram(main_shader);

    int view_proj_loc = glGetUniformLocation(main_shader, "view_proj");

    int model_loc = glGetUniformLocation(main_shader, "model");
    int model_view_proj_loc = glGetUniformLocation(main_shader, "model_view_proj");

    int diffuse_loc = glGetUniformLocation(main_shader, "diffuse_tex");
    int normal_loc = glGetUniformLocation(main_shader, "normal_tex");
    int specular_loc = glGetUniformLocation(main_shader, "specular_tex");

    int ambient_location = glGetUniformLocation(main_shader, "ambient_light");
    int light_dir_location = glGetUniformLocation(main_shader, "light_dir");
    int light_color_location = glGetUniformLocation(main_shader, "light_color");
    int spec_pow_location = glGetUniformLocation(main_shader, "specular_power");

    float sq_3 = sqrt(3.f);

    glUniform3f(ambient_location, 0.2f, 0.2f, 0.2f);
    glUniform3f(light_dir_location, sq_3, -sq_3, sq_3);
    glUniform3f(light_color_location, 0.8f, 0.8f, 0.8f);
    glUniform1f(spec_pow_location, 15.0f);

    glUniformMatrix4fv(view_proj_loc, 1, GL_FALSE, (float*)&view_proj);
    glUniform1i(diffuse_loc, 0);
    glUniform1i(normal_loc, 1);
    glUniform1i(specular_loc, 2);

    for (int i = 0; i < queue_used; ++i)
    {
        RenderItem* item = render_queue + i;
        
        mat4 model_view_proj = view_proj * item->transform;

        glUniformMatrix4fv(model_loc, 1, GL_FALSE, (float*)&item->transform);
        glUniformMatrix4fv(model_view_proj_loc, 1, GL_FALSE, (float*)&model_view_proj);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, item->material->diffuse_texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, item->material->normal_texture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, item->material->specular_texture);
        
        glBindVertexArray(item->mesh->vao);
        glDrawElements(GL_TRIANGLES, item->mesh->index_count, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    queue_used = 0;
}