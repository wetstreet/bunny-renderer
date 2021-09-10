#include "OpenGLRenderer.h"

float rectangleVertices[] =
{
	// Coords    // texCoords
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,

	 1.0f,  1.0f,  1.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f
};

OpenGLRenderer::OpenGLRenderer()
{
	shader = new Shader("src/opengl/shaders/default.vert", "src/opengl/shaders/default.frag");
	postprocessShader = new Shader("src/opengl/shaders/postprocess.vert", "src/opengl/shaders/postprocess.frag");

	glEnable(GL_DEPTH_TEST);

	// Prepare framebuffer rectangle VBO and VAO
	glGenVertexArrays(1, &rectVAO);
	glGenBuffers(1, &rectVBO);
	glBindVertexArray(rectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // framebuffer
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// color rt
	glGenTextures(1, &renderTexture);
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewport.x, viewport.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0); 

    // depth & stencil rbo
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, viewport.x, viewport.y);  
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // postprocess fbo
	glGenFramebuffers(1, &postprocessFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);

	// postprocess color rt
	glGenTextures(1, &postprocessRT);
	glBindTexture(GL_TEXTURE_2D, postprocessRT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewport.x, viewport.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postprocessRT, 0);
}

OpenGLRenderer::~OpenGLRenderer()
{
    delete shader;
    delete postprocessShader;
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

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glEnable(GL_DEPTH_TEST);

    glm::vec3 clearColor = scene.camera->clearColor;
    glClearColor(clearColor.x, clearColor.y, clearColor.z, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene.camera->updateMatrix(45.0f, 0.1f, 1000.0f, viewport.x, viewport.y);

    scene.Draw(shader);

    skybox.Draw(scene.camera);

    // post process
    glBindTexture(GL_TEXTURE_2D, postprocessRT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewport.x, viewport.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);
    glDisable(GL_DEPTH_TEST);

    postprocessShader->Activate();
	glUniform1i(glGetUniformLocation(postprocessShader->ID, "screenTexture"), 0);
    glBindTexture(GL_TEXTURE_2D, renderTexture);

    glBindVertexArray(rectVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // editor background
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.22f, 0.22f, 0.22f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}