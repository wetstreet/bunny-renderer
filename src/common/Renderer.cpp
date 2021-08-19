#include "Renderer.h"

void Renderer::Init_Scene(Camera *camera)
{
	shader = new Shader("src/opengl/shaders/default.vert", "src/opengl/shaders/default.frag");
	scene = new Scene(camera);
	head_diffuse = new Texture("obj/african_head/african_head_diffuse.tga", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
	white_tex = new Texture("obj/white_texture.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);

	Mesh *head = new Mesh("obj/african_head/african_head.obj");
	head->texture = head_diffuse;
	head->shader = shader;
    strcpy(head->name, "head");
	scene->AddObject(head);
}

int Renderer::AddPrimitive(std::string name)
{
	Mesh *mesh = new Mesh(("obj/" + name + ".obj").c_str());
	mesh->texture = white_tex;
	mesh->shader = shader;
    strcpy(mesh->name, name.c_str());
	return scene->AddObject(mesh);
}

void Renderer::Init_OpenGL()
{
	glEnable(GL_DEPTH_TEST);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// generate texture
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 800, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0); 

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 800);  
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
}

void Renderer::Render_OpenGL(GLFWwindow* window, float deltaTime)
{
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewport.x, viewport.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, viewport.x, viewport.y);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glViewport(0, 0, viewport.x, viewport.y);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glClearColor(clear_color.x, clear_color.y, clear_color.z, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene->camera->SceneInputs(window, deltaTime);
    scene->camera->updateMatrix(45.0f, 0.1f, 1000.0f, viewport.x, viewport.y);

    scene->Draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.22f, 0.22f, 0.22f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Render_Rasterizer()
{
    uint8_t* pixels = new uint8_t[ras.size.x * ras.size.y * 3];
    ras.scene = scene;
    ras.width = ras.size.x;
    ras.height = ras.size.y;
    ras.Clear(pixels, clear_color);
    ras.Render(pixels);

    // Create a OpenGL texture identifier
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ras.size.x, ras.size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    delete pixels;
}

Renderer::~Renderer()
{
    delete head_diffuse;
    delete white_tex;

    delete shader;
    delete scene;
}