#include "rasterizer.h"

Model *model = NULL;
Vec3f lightDir(1.0f, 1.0f, 1.0f);
Vec3f eye(1.0f, 1.0f, 3.0f);
Vec3f center(0.0f, 0.0f, 0.0f);

struct GouraudShader : public IShader
{
    mat<2, 3, float> varying_uv;
    mat<3, 3, float> varying_nrm;
    mat<3, 3, float> ndc_tri;

    virtual Vec4f vertex(int iface, int nthvert)
    {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        varying_nrm.set_col(nthvert, proj<3>((Projection * ModelView).invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.0f)));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        ndc_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));
        return Viewport * Projection * ModelView * gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color)
    {
        Vec2f uv = varying_uv * bar;
        Vec3f bn = (varying_nrm * bar).normalize();

        mat<3, 3, float> A;
        A[0] = ndc_tri.col(1) - ndc_tri.col(0);
        A[1] = ndc_tri.col(2) - ndc_tri.col(0);
        A[2] = bn;

        mat<3, 3, float> AI = A.invert();

        Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
        Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

        mat<3, 3, float> B;
        B.set_col(0, i.normalize());
        B.set_col(1, j.normalize());
        B.set_col(2, bn);

        Vec3f n = (B * model->normalmap(uv)).normalize();
        Vec3f l = lightDir;
        Vec3f r = (n * (n * l * 2.0f) - l).normalize();
        float spec = pow(std::max(r.z , 0.0f), model->specular(uv));
        float diff = std::max(n * l, 0.0f);
        TGAColor c = model->diffuse(uv);

        color = c * diff;
        // for (int i = 0; i < 3; i++)
        // {
        //     color[i] = std::min<float>(255, 5 + c[i] * (diff + 0.6 * spec));
        // }
        return false;
    }
};

void Rasterizer::flip_vertically(uint8_t* pixels) {
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

void Rasterizer::Render(uint8_t* pixels, Camera *camera)
{
    model = new Model("obj/african_head/african_head.obj");

    TGAImage image(width, height, TGAImage::RGB);
    image.pixels = pixels;
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    TGAImage diffuse = TGAImage();
    diffuse.read_tga_file("obj/african_head/african_head_diffuse.tga");

    lookat(camera->Position, camera->Position + camera->Orientation, camera->Up);
    projection(-1.0f / (eye - center).norm());
    viewport(0, 0, width, height);
    
    GouraudShader shader;
    for (int i = 0; i < model->nfaces(); i++)
    {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; j++)
        {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }
    
    flip_vertically(image.pixels);

    delete model;
}