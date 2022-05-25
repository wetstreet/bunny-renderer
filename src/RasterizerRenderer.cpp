#include "RasterizerRenderer.h"

void RasterizerRenderer::Render(Scene &scene)
{
    uint8_t* pixels = new uint8_t[viewport.x * viewport.y * 4];
    Rasterize(scene, pixels);

    // Create a OpenGL texture identifier
    glGenTextures(1, &renderTexture);
    glBindTexture(GL_TEXTURE_2D, renderTexture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport.x, viewport.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    delete [] pixels;
}

struct GouraudShader : public IShader
{
    glm::vec3 lightDir;
    glm::mat4 MVP;
    std::shared_ptr<Texture> texture;

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

void RasterizerRenderer::Clear(uint8_t* pixels, glm::vec3 &clearColor)
{
    for (int j = 0; j < viewport.y; j++)
    {
        for (int i = 0; i < viewport.x; i++)
        {
            int index = i + j * viewport.x;
            pixels[index * 4] = clearColor[0] * 255;
            pixels[index * 4 + 1] = clearColor[1] * 255;
            pixels[index * 4 + 2] = clearColor[2] * 255;
            pixels[index * 4 + 3] = 255;
        }
    }
}

void RasterizerRenderer::flip_vertically(uint8_t* pixels)
{
    unsigned long bytes_per_line = viewport.x * 4;
    unsigned char *line = new unsigned char[bytes_per_line];
    int half = viewport.y >> 1;
    for (int j=0; j<half; j++) {
        unsigned long l1 = j*bytes_per_line;
        unsigned long l2 = (viewport.y-1-j)*bytes_per_line;
        memmove((void *)line,      (void *)(pixels+l1), bytes_per_line);
        memmove((void *)(pixels+l1), (void *)(pixels+l2), bytes_per_line);
        memmove((void *)(pixels+l2), (void *)line,      bytes_per_line);
    }
    delete [] line;
}

void RasterizerRenderer::Rasterize(Scene &scene, uint8_t *pixels)
{
    Clear(pixels, scene.camera.clearColor);

    int *zbuffer = new int[viewport.x * viewport.y];
    
    GouraudShader shader;
    shader.lightDir = -scene.camera.Orientation;

    Camera &camera = scene.camera;

    glm::vec3 center = camera.Position + camera.Orientation;
    glm::mat4 View = lookat(camera.Position, center, camera.Up);
    glm::mat4 Projection = projection(-1.0f / glm::length(-camera.Orientation));
    glm::mat4 Viewport = viewportMat(0, 0, viewport.x, viewport.y);
    
    for (int i = 0; i < scene.objects.size(); i++)
    {
        std::shared_ptr<Object> object = scene.objects[i];
        if (object->GetType() == Type_Mesh && object->isEnabled)
        {
            std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(object);

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

                triangle(varys, shader, pixels, zbuffer, viewport.x, viewport.y);
            }
        }
    }

    delete[] zbuffer;
    
    flip_vertically(pixels);
}