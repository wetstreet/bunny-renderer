#include "rasterizer.h"

Model *model = NULL;
Vec3f lightDir(1.0f, 1.0f, 1.0f);

struct GouraudShader : public IShader
{
    mat<2, 3, float> varying_uv;
    mat<3, 3, float> varying_nrm;
    mat<3, 3, float> ndc_tri;

    virtual Vec4f vertex(int iface, int nthvert)
    {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        varying_nrm.set_col(nthvert, proj<3>((Projection * ModelView).invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.0f)));
        // std::cout << "iface=" << iface << ",nthvert=" << nthvert << std::endl;
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        // std::cout << "gl_Vertex=" << gl_Vertex << std::endl;
        ndc_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));
        return Viewport * Projection * ModelView * gl_Vertex;
    }

    virtual Varying vertex_ext(Vertex i, glm::mat4 MVP)
    {
        Varying o;
        o.uv = i.texcoord;
        o.normal = i.normal; // mul(i.normal, object_2_world);
        o.position = MVP * glm::vec4(i.position, 1);
        return o;
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

    virtual glm::vec4 fragment_ext(glm::vec2 uv, Texture *tex)
    {
        int index = uv.x + uv.y * tex->width;
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

    // model = new Model("obj/african_head/african_head.obj");
    // model = new Model("obj/cube.obj");

    int *zbuffer = new int[width*height];

    TGAImage diffuse = TGAImage();
    diffuse.read_tga_file("obj/african_head/african_head_diffuse.tga");
    
    GouraudShader shader;

    Camera *camera = scene->camera;

    // lookat(camera->Position, camera->Position + camera->Orientation, camera->Up);
    // projection(-1.0f / glm::length(-camera->Orientation));
    // viewport(0, 0, width, height);

    // Matrix MVP = Viewport * (Projection * ModelView);
    // std:: cout << "ModelView=" << ModelView << std::endl;
    // std:: cout << "Projection=" << Projection << std::endl;
    // std:: cout << "Viewport=" << Viewport << std::endl;
    // std:: cout << "mvp=" << MVP << std::endl;

    // for (int i = 0; i < model->nfaces(); i++)
    // {
    //     float progress = (float)i / model->nfaces(); 
    //     std::cout << "progress -- " << progress * 100 << "\r";
    //     Vec4f screen_coords[3];
    //     for (int j = 0; j < 3; j++)
    //     {
    //         screen_coords[j] = shader.vertex(i, j);
    //     }
    //     triangle(screen_coords, shader, pixels, zbuffer, width, height);
    // }

    glm::mat4 ModelView = lookat_ext(camera->Position, camera->Position + camera->Orientation, camera->Up);
    glm::mat4 Projection = projection_ext(-1.0f / glm::length(-camera->Orientation));
    glm::mat4 Viewport = viewport_ext(0, 0, width, height);
    glm::mat4 MVP = Projection * ModelView;
    
    for (int i = 0; i < scene->meshes.size(); i++)
    {
        Mesh *mesh = scene->meshes[i];
        
        for (int j = 0; j < mesh->indices.size() / 3; j++)
        {
            Varying varys[3];

            for (int k = 0; k < 3; k++)
            {
                int index = j * 3 + k;
                Vertex vert = mesh->vertices[mesh->indices[index]];
                varys[k] = shader.vertex_ext(vert, MVP);
                varys[k].position = Viewport * varys[k].position;
            }

            triangle_ext(varys, shader, pixels, zbuffer, width, height, mesh->texture);
        }
    }
    
    flip_vertically(pixels);

    delete model;
}