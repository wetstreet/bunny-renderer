#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include "glm/glm.hpp"
#include "../common/Vertex.h"
#include "../common/Texture.h"

struct Varying
{
    glm::vec4 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::mat3 TBN;
};

struct IShader
{
    glm::vec3 lightDir;
    glm::mat4 MVP;
    glm::mat4 objectToWorld;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<Texture> normalMap;

    virtual ~IShader() {}
    virtual Varying vertex(Vertex i) = 0;
    virtual glm::vec4 fragment(Varying &varying) = 0;
};

struct GouraudShader : public IShader
{
    virtual Varying vertex(Vertex i)
    {
        Varying o;
        o.uv = i.texcoord;
        o.normal = objectToWorld * glm::vec4(i.normal, 0);
        o.position = MVP * glm::vec4(i.position, 1);
        return o;
    }

    virtual glm::vec4 fragment(Varying& varying)
    {
        glm::vec4 color = texture->tex2D(varying.uv);

        float nl = std::max(0.0f, glm::dot(varying.normal, lightDir));

        color.r *= nl;
        color.g *= nl;
        color.b *= nl;

        return color;
    }
};

struct NormalShader : public IShader
{
    virtual Varying vertex(Vertex i)
    {
        using namespace glm;

        Varying o;
        o.uv = i.texcoord;
        o.position = MVP * vec4(i.position, 1);

        vec3 aBitangent = cross(i.normal, i.tangent);
        vec3 T = normalize(vec3(objectToWorld * vec4(i.tangent, 0.0)));
        vec3 B = normalize(vec3(objectToWorld * vec4(aBitangent, 0.0)));
        vec3 N = normalize(vec3(objectToWorld * vec4(i.normal, 0.0)));
        o.TBN = mat3(T, B, N);

        return o;
    }

    virtual glm::vec4 fragment(Varying& varying)
    {
        using namespace glm;

        vec4 color = texture->tex2D(varying.uv);

        vec3 normal = normalMap->tex2D(varying.uv);
        normal = normal * 2.0f - 1.0f;
        normal = normalize(varying.TBN * normal);

        float nl = std::max(0.0f, glm::dot(normal, lightDir));

        color.r *= nl;
        color.g *= nl;
        color.b *= nl;

        return color;
    }
};

glm::mat4 viewportMat(int x, int y, int w, int h)
{
    glm::mat4 Viewport = glm::mat4(1);
    Viewport[0][3] = x + w / 2.0f;
    Viewport[1][3] = y + h / 2.0f;
    Viewport[2][3] = 1 / 2.0f;

    Viewport[0][0] = w / 2.0f;
    Viewport[1][1] = h / 2.0f;
    Viewport[2][2] = 1 / 2.0f;
    return glm::transpose(Viewport);
}

glm::vec3 barycentric(glm::vec3& A, glm::vec3& B, glm::vec3& C, glm::ivec2& P)
{
    glm::vec3 a = glm::vec3(B.x - A.x, C.x - A.x, A.x - P.x);
    glm::vec3 b = glm::vec3(B.y - A.y, C.y - A.y, A.y - P.y);
    glm::vec3 u = cross(a, b);

    if (std::abs(u[2]) < 1)
        return glm::vec3(-1, 1, 1);

    return glm::vec3(1.0f - (u.x + u.y) / u.z, u.x / u.z, u.y / u.z);
}

void triangle(Varying* varys, IShader& shader, uint8_t* pixels, float* zbuffer, int width, int height)
{
    glm::vec2 bboxmin = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    glm::vec2 bboxmax = glm::vec2(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin[j] = std::min(varys[i].position[j] / varys[i].position[3], bboxmin[j]);
            bboxmax[j] = std::max(varys[i].position[j] / varys[i].position[3], bboxmax[j]);
        }
    }
    glm::ivec2 P;
    for (P.x = bboxmin.x; P.x < bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y < bboxmax.y; P.y++)
        {
            if (P.x < 0 || P.y < 0 || P.x >= width || P.y >= height) continue;

            glm::vec3 A(varys[0].position / varys[0].position[3]);
            glm::vec3 B(varys[1].position / varys[1].position[3]);
            glm::vec3 C(varys[2].position / varys[2].position[3]);
            glm::vec3 c = barycentric(A, B, C, P);

            if (c.x < 0 || c.y < 0 || c.z < 0) continue;

            // depth interpolation
            // ref:https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf (equation 12)
            float depth = 1 / (c.x / A.z + c.y / B.z + c.z / C.z);
            //float depth = c.x * A.z + c.y * B.z + c.z * C.z;

            if (depth < 0 || depth > 1) continue;

            int index = P.x + P.y * width;

            if (depth > zbuffer[index]) continue;

            // perspective correct attributes interpolation
            // ref:https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf (page 58 equation 3.5)
            float weight0 = c.x / varys[0].position[3];
            float weight1 = c.y / varys[1].position[3];
            float weight2 = c.z / varys[2].position[3];
            float denom = 1 / (weight0 + weight1 + weight2);

            Varying o;
            float size = sizeof(Varying) / sizeof(float);
            float* floats_o = (float*)&o;
            float* floats_i0 = (float*)&varys[0];
            float* floats_i1 = (float*)&varys[1];
            float* floats_i2 = (float*)&varys[2];
            // 0~3 is position, so index starts from 4
            for (int i = 4; i < size; i++)
            {
                floats_o[i] = (floats_i0[i] * weight0 + floats_i1[i] * weight1 + floats_i2[i] * weight2) * denom;
            }

            glm::vec4 color = shader.fragment(o);

            pixels[index * 4] = color.r * 255;
            pixels[index * 4 + 1] = color.g * 255;
            pixels[index * 4 + 2] = color.b * 255;
            pixels[index * 4 + 3] = 255;

            zbuffer[index] = depth;
        }
    }
}


#endif //__OUR_GL_H__