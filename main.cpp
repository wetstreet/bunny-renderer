#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

const int width = 800;
const int height = 800;

Model *model = NULL;
Vec3f lightDir(1, 1, 1);
Vec3f eye(1,1,3);
Vec3f center(0,0,0);

struct GouraudShader : public IShader
{
    mat<2, 3, float> varying_uv;
    mat<4, 4, float> uniform_M;     // Projection * ModelView
    mat<4, 4, float> uniform_MIT;   // (Projection * ModelView).invert_transpose()

    virtual Vec4f vertex(int iface, int nthvert)
    {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        return Viewport * Projection * ModelView * gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color)
    {
        Vec2f uv = varying_uv * bar;
        Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normalmap(uv))).normalize();
        Vec3f l = proj<3>(uniform_M * embed<4>(lightDir)).normalize();
        Vec3f r = (n * (n * l * 2.0f) - l).normalize();
        float spec = pow(std::max(r.z , 0.0f), model->specular(uv));
        float diff = std::max(n * l, 0.0f);
        TGAColor c = model->diffuse(uv);
        for (int i = 0; i < 3; i++)
        {
            color[i] = std::min<float>(255, 5 + c[i] * (diff + 0.6 * spec));
        }
        return false;
    }
};

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        model = new Model(argv[1]);
    }
    else
    {
        model = new Model("obj/african_head/african_head.obj");
        // model = new Model("obj/cube.obj");
        // model = new Model("obj/icosphere.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    TGAImage diffuse = TGAImage();
    diffuse.read_tga_file("obj/african_head/african_head_diffuse.tga");


    lookat(eye, center, Vec3f(0, 1, 0));
    projection(-1.0f / (eye - center).norm());
    viewport(0, 0, width, height);
    
    GouraudShader shader;
    shader.uniform_M = Projection * ModelView;
    shader.uniform_MIT = (Projection * ModelView).invert_transpose();
    for (int i = 0; i < model->nfaces(); i++)
    {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; j++)
        {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }
    

    std::cout << "finish";

    image.flip_vertically();
    zbuffer.flip_vertically();
    image.write_tga_file("output.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    delete model;
    return 0;
}