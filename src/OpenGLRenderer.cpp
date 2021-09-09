#include "OpenGLRenderer.h"

OpenGLRenderer::OpenGLRenderer()
{
	shader = new Shader("src/opengl/shaders/default.vert", "src/opengl/shaders/default.frag");

	glEnable(GL_DEPTH_TEST);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// generate texture
	glGenTextures(1, &renderTexture);
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 800, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0); 

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 800);  
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
}

OpenGLRenderer::~OpenGLRenderer()
{
    delete shader;
}

void OpenGLRenderer::Render(Scene &scene)
{
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewport.x, viewport.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, viewport.x, viewport.y);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glViewport(0, 0, viewport.x, viewport.y);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glm::vec3 clearColor = scene.camera->clearColor;
    glClearColor(clearColor.x, clearColor.y, clearColor.z, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene.camera->updateMatrix(45.0f, 0.1f, 1000.0f, viewport.x, viewport.y);

    scene.Draw(shader);

    skybox.Draw(scene.camera);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.22f, 0.22f, 0.22f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}