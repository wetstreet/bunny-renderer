#ifndef __RASTERIZER_RENDERER_H__
#define __RASTERIZER_RENDERER_H__

#include <vector>
#include <iostream>

#include "common/Renderer.h"
#include "common/Utils.h"
#include "rasterizer/our_gl.h"

class RasterizerRenderer : public Renderer
{
    public:
        GLuint depthTexture;

        RasterizerRenderer()
        {
            glGenTextures(1, &renderTexture);
            glBindTexture(GL_TEXTURE_2D, renderTexture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glGenTextures(1, &depthTexture);
            glBindTexture(GL_TEXTURE_2D, depthTexture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        virtual void RasterizerRenderer::Render(Scene& scene)
        {
            int length = viewport.x * viewport.y;

            uint8_t* pixels = new uint8_t[length * 4];
            float* zbuffer = new float[length];
            std::fill(zbuffer, zbuffer + length, 1.0f);

            Rasterize(scene, pixels, zbuffer);

            glBindTexture(GL_TEXTURE_2D, renderTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport.x, viewport.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

            glBindTexture(GL_TEXTURE_2D, depthTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, viewport.x, viewport.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, zbuffer);

            delete[] zbuffer;
            delete[] pixels;
        }
        
    private:

        void RasterizerRenderer::Clear(uint8_t* pixels, glm::vec3& clearColor)
        {
            for (int j = 0; j < viewport.y; j++)
            {
                for (int i = 0; i < viewport.x; i++)
                {
                    int index = i + j * viewport.x;
                    pixels[index * 4] = uint8_t(clearColor[0] * 255);
                    pixels[index * 4 + 1] = uint8_t(clearColor[1] * 255);
                    pixels[index * 4 + 2] = uint8_t(clearColor[2] * 255);
                    pixels[index * 4 + 3] = 255;
                }
            }
        }

        void RemapZBuffer(float* zbuffer, int width, int height)
        {
            int length = width * height;
            // find minimal zbuffer value
            float min = 1.0f;
            for (int i = 0; i < length; i++)
            {
                if (zbuffer[i] < min)
                    min = zbuffer[i];
            }
            // map from [0,1] to [min,1]
            for (int i = 0; i < length; i++)
            {
                zbuffer[i] = (zbuffer[i] - min) / (1 - min);
            }
        }

        void RasterizerRenderer::Rasterize(Scene& scene, uint8_t* pixels, float* zbuffer)
        {
            Clear(pixels, scene.camera.clearColor);

            Camera& camera = scene.camera;

            glm::vec3 center = camera.Position + camera.Orientation;
            glm::mat4 Viewport = viewportMat(0, 0, viewport.x, viewport.y);

            glm::vec3 lightPos;
            glm::vec3 lightColor;
            scene.GetMainLightProperties(lightPos, lightColor);

            for (int i = 0; i < scene.objects.size(); i++)
            {
                std::shared_ptr<Object> object = scene.objects[i];
                if (object->GetType() == Type_Mesh && object->isEnabled)
                {
                    std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(object);

                    mesh->material->MVP = camera.cameraMatrix * mesh->objectToWorld;
                    mesh->material->objectToWorld = mesh->objectToWorld;
                    mesh->material->worldToObject = mesh->worldToObject;
                    mesh->material->lightDir = lightPos;
                    mesh->material->lightColor = lightColor;

                    for (int j = 0; j < mesh->indices.size() / 3; j++)
                    {
                        Varying varys[3];

                        for (int k = 0; k < 3; k++)
                        {
                            int index = j * 3 + k;
                            Vertex vert = mesh->vertices[mesh->indices[index]];
                            varys[k] = mesh->material->vertex(vert);
                            varys[k].position = Viewport * varys[k].position;
                        }

                        // todo : cull back face
                        rasterize_triangle(varys, *mesh->material, pixels, zbuffer, viewport.x, viewport.y);
                    }
                }
            }

            RemapZBuffer(zbuffer, viewport.x, viewport.y);
        }
};

#endif //__RASTERIZER_RENDERER_H__