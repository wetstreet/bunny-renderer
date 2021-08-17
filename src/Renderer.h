#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "opengl/Scene.h"
#include "opengl/Texture.h"
#include "rasterizer/rasterizer.h"

class Renderer
{
    public:
        static Renderer GetInstance()
        {
            static Renderer renderer;
            return renderer;
        }
        void Init_Scene(Camera *camera);
        void Init_OpenGL();
        void Render_OpenGL(GLFWwindow* window, float deltaTime);
        void Render_Rasterizer();
        void Delete();

    private:
        Renderer() {}
        Texture *head_diffuse;
        Texture *white_tex;

    public:
        glm::vec2 viewport = glm::vec2(800, 800);
        glm::vec3 clear_color = glm::vec3(0.45f, 0.55f, 0.60f);
	    GLuint texColorBuffer;
        GLuint rbo;
        GLuint framebuffer;
	    GLuint image_texture;
        Shader *shader;
        Scene *scene;
	    Rasterizer ras;
};

#endif //__RENDERER_H__