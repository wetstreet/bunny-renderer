#include "rasterizer.h"

// Vec3f lightDir(1.0f, 1.0f, 1.0f);

struct GouraudShader : public IShader
{
    virtual Varying vertex(Vertex i, glm::mat4 MVP)
    {
        Varying o;
        o.uv = i.texcoord;
        o.normal = i.normal; // mul(i.normal, object_2_world);
        o.position = MVP * glm::vec4(i.position, 1);
        return o;
    }

    virtual glm::vec4 fragment(glm::vec2 uv, Texture *tex)
    {
        glm::ivec2 intuv(uv.x * tex->width, uv.y * tex->height);
        int index = intuv.x + intuv.y * tex->width;
        glm::vec3 color = glm::vec3(tex->bytes[index*3]/255.0f, tex->bytes[index*3+1]/255.0f, tex->bytes[index*3+2]/255.0f);

        return glm::vec4(color, 1);
    }
};

void Rasterizer::Clear(uint8_t* pixels)
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int index = i + j * width;
            pixels[index * 3] = 0;
            pixels[index * 3 + 1] = 0;
            pixels[index * 3 + 2] = 0;
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
    Clear(pixels);

    int *zbuffer = new int[width*height];
    
    GouraudShader shader;

    Camera *camera = scene->camera;

    glm::mat4 View = lookat(camera->Position, camera->Position + camera->Orientation, camera->Up);
    glm::mat4 Projection = projection(-1.0f / glm::length(-camera->Orientation));
    glm::mat4 Viewport = viewport(0, 0, width, height);
    
    for (int i = 0; i < scene->meshes.size(); i++)
    {
        Mesh *mesh = scene->meshes[i];
        glm::mat4 Model = model_matrix(mesh->position, mesh->scale);
        glm::mat4 MVP = Projection * View * Model;
        
        for (int j = 0; j < mesh->indices.size() / 3; j++)
        {
            Varying varys[3];

            for (int k = 0; k < 3; k++)
            {
                int index = j * 3 + k;
                Vertex vert = mesh->vertices[mesh->indices[index]];
                varys[k] = shader.vertex(vert, MVP);
                varys[k].position = Viewport * varys[k].position;
            }

            triangle(varys, shader, pixels, zbuffer, width, height, mesh->texture);
        }
    }
    
    flip_vertically(pixels);
}