#include <cmath>
#include <limits>
#include <cstdlib>
#include "our_gl.h"
#include "../common/Mesh.h"

IShader::~IShader() {}

glm::mat4 rotate(glm::mat4 &m, float angle, glm::vec3 const &v)
{
    float const a = angle;
    float const c = cos(a);
    float const s = sin(a);

    glm::vec3 axis(glm::normalize(v));
    glm::vec3 temp((1.0f - c) * axis);

    glm::mat4 Rotate;
    Rotate[0][0] = c + temp[0] * axis[0];
    Rotate[0][1] = temp[0] * axis[1] + s * axis[2];
    Rotate[0][2] = temp[0] * axis[2] - s * axis[1];

    Rotate[1][0] = temp[1] * axis[0] - s * axis[2];
    Rotate[1][1] = c + temp[1] * axis[1];
    Rotate[1][2] = temp[1] * axis[2] + s * axis[0];

    Rotate[2][0] = temp[2] * axis[0] + s * axis[1];
    Rotate[2][1] = temp[2] * axis[1] - s * axis[0];
    Rotate[2][2] = c + temp[2] * axis[2];

    glm::mat4 Result;
    Result[0] = m[0] * Rotate[0][0] + m[1] * Rotate[0][1] + m[2] * Rotate[0][2];
    Result[1] = m[0] * Rotate[1][0] + m[1] * Rotate[1][1] + m[2] * Rotate[1][2];
    Result[2] = m[0] * Rotate[2][0] + m[1] * Rotate[2][1] + m[2] * Rotate[2][2];
    Result[3] = m[3];
    return Result;
}

glm::mat4 model_matrix(glm::vec3 &position, glm::vec3 &rotation, glm::vec3 &scale)
{
	glm::mat4 objectToWorld = glm::mat4(1);

	for (int i = 0; i < 3; i++)
	{
		objectToWorld = rotate(objectToWorld, rotation[i] * DEG2RAD, directionUnary[i]);
	}

	float validScale[3];
	for (int i = 0; i < 3; i++)
	{
		if (fabsf(scale[i]) < FLT_EPSILON)
		{
		    validScale[i] = 0.001f;
		}
		else
		{
		    validScale[i] = scale[i];
		}
	}
	objectToWorld[0] *= validScale[0];
	objectToWorld[1] *= validScale[1];
	objectToWorld[2] *= validScale[2];
	objectToWorld[3] = glm::vec4(position[0], position[1], position[2], 1.f);
    return objectToWorld;
}

glm::mat4 lookat(glm::vec3 &eye, glm::vec3 &center, glm::vec3 &up)
{
    glm::vec3 z = glm::normalize(eye - center);
    glm::vec3 x = glm::normalize(cross(up, z));
    glm::vec3 y = glm::normalize(cross(z, x));
    glm::mat4 View = glm::mat4(1);
    for (int i=0; i<3; i++)
    {
        View[0][i] = x[i];
        View[1][i] = y[i];
        View[2][i] = z[i];
        View[i][3] = -center[i];
    }
    return glm::transpose(View);
}

glm::mat4 projection(float coeff)
{
    glm::mat4 Projection = glm::mat4(1);
    Projection[3][2] = coeff;
    return glm::transpose(Projection);
}

const int depth  = 255;

glm::mat4 viewportMat(int x, int y, int w, int h)
{
    glm::mat4 Viewport = glm::mat4(1);
    Viewport[0][3] = x + w / 2.0f;
    Viewport[1][3] = y + h / 2.0f;
    Viewport[2][3] = depth / 2.0f;

    Viewport[0][0] = w / 2.0f;
    Viewport[1][1] = h / 2.0f;
    Viewport[2][2] = depth / 2.0f;
    return glm::transpose(Viewport);
}

glm::vec3 barycentric(glm::vec2 &A, glm::vec2 &B, glm::vec2 &C, glm::ivec2 &P)
{
    glm::vec3 a = glm::vec3(B.x - A.x, C.x - A.x, A.x - P.x);
    glm::vec3 b = glm::vec3(B.y - A.y, C.y - A.y, A.y - P.y);
    glm::vec3 u = cross(a, b);

    if (std::abs(u[2]) < 1)
        return glm::vec3(-1, 1, 1);

    return glm::vec3(1.0f - (u.x + u.y)/u.z, u.x/u.z, u.y/u.z);
}

glm::vec3 tex2D(Texture &tex, glm::vec2 &uv)
{
    glm::ivec2 intuv(uv.x * tex.width, uv.y * tex.height);
    int index = intuv.x + intuv.y * tex.width;
    return glm::vec3(tex.bytes[index*3]/255.0f, tex.bytes[index*3+1]/255.0f, tex.bytes[index*3+2]/255.0f);
}

void triangle(Varying *varys, IShader &shader, uint8_t* pixels, float*zbuffer, int width, int height, Camera& camera)
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
            
            glm::vec2 A(varys[0].position / varys[0].position[3]);
            glm::vec2 B(varys[1].position / varys[1].position[3]);
            glm::vec2 C(varys[2].position / varys[2].position[3]);
            glm::vec3 c = barycentric(A, B, C, P);

            float z = varys[0].position[2] * c.x + varys[1].position[2] * c.y + varys[2].position[2] * c.z;
            float w = varys[0].position[3] * c.x + varys[1].position[3] * c.y + varys[2].position[3] * c.z;
            float depth = std::max(0.0f, std::min((z / w - camera.nearPlane) / (camera.farPlane - camera.nearPlane), 1.0f));

            int index = P.x + P.y * width;

            if (c.x < 0 || c.y < 0 || c.z < 0 || zbuffer[index] > depth) continue;

            Varying o;
            o.uv = glm::vec2(0, 0);
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    o.uv[i] += c[j] * varys[j].uv[i];
                }
            }
            o.normal = glm::vec3(0, 0, 0);
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    o.normal[i] += c[j] * varys[j].normal[i];
                }
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
