#include "rasterizer.h"

// Vec3f lightDir(1.0f, 1.0f, 1.0f);

struct GouraudShader : public IShader
{
    glm::vec3 lightDir;
    glm::mat4 MVP;
    Texture *texture;

    virtual Varying vertex(Vertex i)
    {
        Varying o;
        o.uv = i.texcoord;
        o.normal = i.normal; // mul(i.normal, object_2_world);
        o.position = MVP * glm::vec4(i.position, 1);
        return o;
    }

    virtual glm::vec4 fragment(Varying &varying)
    {
        glm::vec3 color = tex2D(*texture, varying.uv);

        float nl = std::max(0.0f, glm::dot(varying.normal, lightDir));

        return glm::vec4(color * nl, 1);
    }
};

void Rasterizer::Clear(uint8_t* pixels, glm::vec3 &clearColor)
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int index = i + j * width;
            pixels[index * 3] = clearColor[0] * 255;
            pixels[index * 3 + 1] = clearColor[1] * 255;
            pixels[index * 3 + 2] = clearColor[2] * 255;
        }
    }
}

void Rasterizer::flip_vertically(uint8_t* pixels)
{
    unsigned long bytes_per_line = width * 3;
    unsigned char *line = new unsigned char[bytes_per_line];
    int half = height>>1;
    for (int j=0; j<half; j++) {
        unsigned long l1 = j*bytes_per_line;
        unsigned long l2 = (height-1-j)*bytes_per_line;
        memmove((void *)line,      (void *)(pixels+l1), bytes_per_line);
        memmove((void *)(pixels+l1), (void *)(pixels+l2), bytes_per_line);
        memmove((void *)(pixels+l2), (void *)line,      bytes_per_line);
    }
    delete [] line;
}

void Rasterizer::Render(uint8_t* pixels)
{
    int *zbuffer = new int[width*height];
    
    GouraudShader shader;
    shader.lightDir = -scene->camera->Orientation;

    Camera *camera = scene->camera;

    glm::vec3 center = camera->Position + camera->Orientation;
    glm::mat4 View = lookat(camera->Position, center, camera->Up);
    glm::mat4 Projection = projection(-1.0f / glm::length(-camera->Orientation));
    glm::mat4 Viewport = viewport(0, 0, width, height);
    
    for (int i = 0; i < scene->meshes.size(); i++)
    {
        Mesh *mesh = scene->meshes[i];

        if (!mesh->isEnabled) continue;
        
        glm::mat4 Model = model_matrix(mesh->position, mesh->rotation, mesh->scale);
        glm::mat4 MVP = Projection * View * Model;
        shader.MVP = MVP;
        shader.texture = mesh->texture;
        
        for (int j = 0; j < mesh->indices.size() / 3; j++)
        {
            Varying varys[3];

            for (int k = 0; k < 3; k++)
            {
                int index = j * 3 + k;
                Vertex vert = mesh->vertices[mesh->indices[index]];
                varys[k] = shader.vertex(vert);
                varys[k].position = Viewport * varys[k].position;
            }

            triangle(varys, shader, pixels, zbuffer, width, height);
        }
    }
    
    flip_vertically(pixels);
}