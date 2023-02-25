#include "OpenGLRenderer.h"
#include "common/Selection.h"

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
	shadowMapShader = std::make_shared<Shader>("editor/shadowMap");
	postprocessShader = std::make_shared<Shader>("editor/postprocess");
	outlineShader = std::make_shared<Shader>("editor/outline");
	outlineCompareShader = std::make_shared<Shader>("editor/outlineCompareIds");
	outlineBlurShader = std::make_shared<Shader>("editor/outlineBlur");
	outlineMergeShader = std::make_shared<Shader>("editor/outlineMerge");

	equirectangularShader = std::make_shared<Shader>("editor/equirectangular");
	irradianceShader = std::make_shared<Shader>("editor/irradiance");
	prefilterShader = std::make_shared<Shader>("editor/prefilter");

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// initialize shadow map
	glGenFramebuffers(1, &shadowMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);                                      

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

	// color rt
	glGenTextures(1, &renderTexture);
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glObjectLabel(GL_TEXTURE, renderTexture, -1, "Color RT");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport.x, viewport.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// framebuffer
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

	// object id rt
	glGenTextures(1, &objectIdRT);
	glBindTexture(GL_TEXTURE_2D, objectIdRT);
	glObjectLabel(GL_TEXTURE, objectIdRT, -1, "Object ID RT");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, viewport.x, viewport.y, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, objectIdRT, 0);

    // depth & stencil rbo
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glObjectLabel(GL_RENDERBUFFER, rbo, -1, "Depth Stencil Buffer");
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, viewport.x, viewport.y);  
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(2, buffers);

	// outlinert
	glGenTextures(1, &outlineRT);
	glBindTexture(GL_TEXTURE_2D, outlineRT);
	glObjectLabel(GL_TEXTURE, outlineRT, -1, "Outline RT");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewport.x, viewport.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// outline fbo
	glGenFramebuffers(1, &outlineFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, outlineFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outlineRT, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);


	// postprocess color rt
	glGenTextures(1, &postprocessRT);
	glBindTexture(GL_TEXTURE_2D, postprocessRT);
	glObjectLabel(GL_TEXTURE, postprocessRT, -1, "PostProcess RT");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewport.x, viewport.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// postprocess fbo
	glGenFramebuffers(1, &postprocessFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postprocessRT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// capture fbo
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	// environment map
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// irradiance map
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// prefilter map
	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// brdf lut
	glGenTextures(1, &brdfLUTTexture);
	glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

OpenGLRenderer::~OpenGLRenderer()
{
}

void OpenGLRenderer::RegisterTexture(Texture* texture)
{
	glGenTextures(1, &texture->ID);
	glBindTexture(GL_TEXTURE_2D, texture->ID);

	glObjectLabel(GL_TEXTURE, texture->ID, -1, texture->name.c_str());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture->mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture->wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture->wrap);

	GLenum format = texture->numColCh == 4 ? GL_RGBA : GL_RGB;

	if (texture->data)
		glTexImage2D(GL_TEXTURE_2D, 0, texture->numColCh == 4 ? GL_RGBA16F : GL_RGB16F, texture->width, texture->height, 0, format, texture->type, texture->data);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, texture->numColCh == 4 ? GL_RGBA8 : GL_RGB8, texture->width, texture->height, 0, format, texture->type, texture->bytes);

	if (texture->mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLRenderer::UnregisterTexture(Texture* texture)
{
	glDeleteTextures(1, &texture->ID);
}

void OpenGLRenderer::BindTexture(Texture& texture, GLuint slot = 0)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texture.ID);
}

void OpenGLRenderer::RegisterSkybox(Skybox* skybox)
{
	GLuint skyboxVBO, skyboxEBO;

	// Create VAO, VBO, and EBO for the skybox
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteBuffers(1, &skyboxEBO);

	// Creates the cubemap texture object
	glGenTextures(1, &cubemapTexture);
	glObjectLabel(GL_TEXTURE, cubemapTexture, -1, "skybox");
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// These are very important to prevent seams
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// This might help with seams on some systems
	//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	for (unsigned int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, skybox->width, skybox->height, 0, GL_RGB, GL_UNSIGNED_BYTE, skybox->textures[i]);
}

void OpenGLRenderer::UnregisterSkybox(Skybox* skybox)
{
	glDeleteVertexArrays(1, &skyboxVAO);
}

void OpenGLRenderer::BindSkybox(Skybox* skybox, GLuint slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
}

void OpenGLRenderer::DrawSkybox()
{
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);

	glBindVertexArray(skyboxVAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glDepthFunc(GL_LESS);
}

glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
glm::mat4 captureViews[] =
{
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
};

void OpenGLRenderer::GenerateCubemapFromEquirectangular(Scene& scene)
{
	equirectangularShader->Activate();
	equirectangularShader->SetUniform("projection", captureProjection);
	equirectangularShader->SetUniform("equirectangularMap", 0);
	BindTexture(*scene.equirectangular);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);

	glViewport(0, 0, 512, 512);
	for (unsigned int i = 0; i < 6; ++i)
	{
		equirectangularShader->SetUniform("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		DrawSkybox();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	scene.skybox.textures_f = new float* [6];


	for (int i = 0; i < 6; i++)
	{
		scene.skybox.textures_f[i] = new float[512 * 512 * 3];
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, GL_FLOAT, scene.skybox.textures_f[i]);

		
		std::stringstream ss;
		ss << "pos_" << i << ".hdr";
		std::string path;
		ss >> path;
		stbi_write_hdr(path.c_str(), 512, 512, 3, scene.skybox.textures_f[i]);
	}

}

void OpenGLRenderer::GenerateIrradianceMap(Scene& scene)
{
	irradianceShader->Activate();
	irradianceShader->SetUniform("environmentMap", 0);
	irradianceShader->SetUniform("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	glViewport(0, 0, 32, 32);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		irradianceShader->SetUniform("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		DrawSkybox();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLRenderer::GeneratePrefilterMap(Scene& scene)
{
	prefilterShader->Activate();
	prefilterShader->SetUniform("environmentMap", 0);
	prefilterShader->SetUniform("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = 128u * std::pow(0.5, mip);
		unsigned int mipHeight = mipWidth;
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilterShader->SetUniform("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilterShader->SetUniform("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			DrawSkybox();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLRenderer::GenerateBrdfLUT(Scene& scene)
{
	// this function only needs to be called once.
	Shader brdfShader("editor/brdf");
	brdfShader.Activate();

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

	glViewport(0, 0, 512, 512);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(rectVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int OpenGLRenderer::GetObjectID(int x, int y)
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glReadBuffer(GL_COLOR_ATTACHMENT1);
	int pixelData;
	glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return pixelData;
}

void LinkAttrib(VBO& vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizei stride, void* offset)
{
	vbo.Bind();
	glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(layout);
	vbo.Unbind();
}

void OpenGLRenderer::RegisterMesh(Mesh* mesh)
{
	glGenVertexArrays(1, &mesh->ID);
	glBindVertexArray(mesh->ID); // must bind before vbo and ebo creation

	VBO vbo(mesh->vertices);
	EBO ebo(mesh->indices);

	LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), 0);
	LinkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	LinkAttrib(vbo, 2, 2, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
    LinkAttrib(vbo, 3, 3, GL_FLOAT, sizeof(Vertex), (void*)(8 * sizeof(float)));
	glBindVertexArray(0);
	vbo.Unbind();
	ebo.Unbind();
}

void OpenGLRenderer::UnregisterMesh(Mesh* mesh)
{
	glDeleteVertexArrays(1, &mesh->ID);
}

void OpenGLRenderer::DrawMesh(Mesh& mesh)
{
	glBindVertexArray(mesh.ID);
	glDrawElements(GL_TRIANGLES, (GLsizei)mesh.indices.size(), GL_UNSIGNED_INT, 0);
}

void OpenGLRenderer::DrawScene(Scene& scene)
{
	glm::vec3 lightPos;
	glm::vec3 lightColor;
	scene.GetMainLightProperties(lightPos, lightColor);

	glm::mat4 lightProjection = scene.GetLightMatrix();

	for (int i = 0; i < scene.objects.size(); i++)
	{
		std::shared_ptr<Object> object = scene.objects[i];
		if (object->GetType() == Type_Mesh)
		{
			std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(object);
			if (mesh->isEnabled)
			{
				if (mesh->material->doubleSided)
					glDisable(GL_CULL_FACE);
				else
				{
					glEnable(GL_CULL_FACE);
					glCullFace(GL_BACK);
				}

				// per material setup
				mesh->material->Setup();

				// common setup
				mesh->material->SetUniform("_ObjectID", i);
				mesh->material->SetUniform("_MainLightPosition", lightPos);
				mesh->material->SetUniform("_MainLightColor", lightColor);
				mesh->material->SetUniform("_WorldSpaceCameraPos", scene.camera.Position);
				mesh->material->SetUniform("_AmbientColor", scene.ambientColor);
				mesh->material->SetUniform("br_ObjectToClip", scene.camera.cameraMatrix * mesh->objectToWorld);
				mesh->material->SetUniform("br_ObjectToWorld", mesh->objectToWorld);
				mesh->material->SetUniform("br_WorldToObject", mesh->worldToObject);
				mesh->material->SetUniform("lightProjection", lightProjection);

				mesh->material->SetUniform("shadowMap", 3);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, shadowMap);

				mesh->material->SetUniform("irradianceMap", 4);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

				mesh->material->SetUniform("prefilterMap", 5);
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

				mesh->material->SetUniform("brdfLUT", 6);
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

				mesh->material->SetUniform("environmentMap", 7);
				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

				DrawMesh(*mesh);
			}
		}
	}
}

void OpenGLRenderer::Render(Scene &scene)
{
	scene.UpdateMatrices();

	// render shadow map
	shadowMapShader->Activate();
	glUniformMatrix4fv(glGetUniformLocation(shadowMapShader->ID, "lightProjection"), 1, GL_FALSE, glm::value_ptr(scene.GetLightMatrix()));

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glViewport(0, 0, shadowMapWidth, shadowMapHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	// render shadowmap
	for (int i = 0; i < scene.objects.size(); i++)
	{
		std::shared_ptr<Object> object = scene.objects[i];
		if (object->GetType() == Type_Mesh)
		{
			std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(object);
			if (mesh->isEnabled)
			{
				glUniformMatrix4fv(glGetUniformLocation(shadowMapShader->ID, "br_ObjectToWorld"), 1, GL_FALSE, glm::value_ptr(mesh->objectToWorld));

				DrawMesh(*mesh);
			}
		}
	}

	scene.shadowMap = shadowMap;

	// update rt size
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport.x, viewport.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, objectIdRT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, viewport.x, viewport.y, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, postprocessRT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport.x, viewport.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, outlineRT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport.x, viewport.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
    
	// update zbuffer size
    glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, viewport.x, viewport.y);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// starts render
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glViewport(0, 0, viewport.x, viewport.y);
	glScissor(0, 0, viewport.x, viewport.y);

	glm::vec3 clearColor = scene.camera.clearColor;
	int noID = -1;

	glDepthMask(GL_TRUE);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearTexImage(objectIdRT, 0, GL_RED_INTEGER, GL_INT, &noID);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

    scene.camera.updateMatrix(viewport.x, viewport.y);

	DrawScene(scene);

	Shader::skyboxShader->Activate();
	Shader::skyboxShader->SetUniform("view", scene.camera.view);
	Shader::skyboxShader->SetUniform("projection", scene.camera.projection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap); // equirectangular
	//glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture); // 6 faces
	DrawSkybox();

	if (node_clicked != -1)
	{
		// disable zwrite
		glDepthMask(GL_FALSE);

		// draw outline
		glBindFramebuffer(GL_FRAMEBUFFER, outlineFBO);

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-0.02f, 0.0f);

		glEnable(GL_DEPTH_TEST);

		std::shared_ptr<Object> selected = scene.objects[node_clicked];
		if (selected->GetType() == Type_Mesh)
		{
			std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(selected);
			outlineShader->Activate();

			glm::vec4 draw_color(1 / 255.0f, 1, 1, 1);
			glUniform4fv(glGetUniformLocation(outlineShader->ID, "_DRAW_COLOR"), 1, (float*)&draw_color);
			glUniformMatrix4fv(glGetUniformLocation(outlineShader->ID, "br_ObjectToClip"), 1, GL_FALSE, glm::value_ptr(scene.camera.cameraMatrix * mesh->objectToWorld));

			DrawMesh(*mesh);
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_MAX);
		glDisable(GL_DEPTH_TEST);

		if (selected->GetType() == Type_Mesh)
		{
			std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(selected);
			outlineShader->Activate();

			glm::vec4 draw_color(0, 0, 1, 1);
			glUniform4fv(glGetUniformLocation(outlineShader->ID, "_DRAW_COLOR"), 1, (float*)&draw_color);
			glUniformMatrix4fv(glGetUniformLocation(outlineShader->ID, "br_ObjectToClip"), 1, GL_FALSE, glm::value_ptr(scene.camera.cameraMatrix * mesh->objectToWorld));

			DrawMesh(*mesh);
		}

		glDisable(GL_BLEND);
		glDisable(GL_POLYGON_OFFSET_FILL);

		// compare id (use post process rt to ping pong)
		glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);
		outlineCompareShader->Activate();
		glUniform1i(glGetUniformLocation(outlineCompareShader->ID, "_MainTex"), 0);
		glBindTexture(GL_TEXTURE_2D, outlineRT);
		glm::vec4 texelSize(1.0f / viewport.x, 1.0f / viewport.y, viewport.x, viewport.y);
		glUniform4fv(glGetUniformLocation(outlineCompareShader->ID, "_MainTex_TexelSize"), 1, (float*)&texelSize);
		glBindVertexArray(rectVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// blur horizontally (use outline rt to ping pong)
		glBindFramebuffer(GL_FRAMEBUFFER, outlineFBO);
		outlineBlurShader->Activate();
		glUniform1i(glGetUniformLocation(outlineBlurShader->ID, "_MainTex"), 0);
		glBindTexture(GL_TEXTURE_2D, postprocessRT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glUniform4fv(glGetUniformLocation(outlineBlurShader->ID, "_MainTex_TexelSize"), 1, (float*)&texelSize);
		glm::vec2 blurDirection(1.0f, 0.0f);
		glUniform2fv(glGetUniformLocation(outlineBlurShader->ID, "_BlurDirection"), 1, (float*)&blurDirection);
		glBindVertexArray(rectVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// blur vertically (use outline rt to ping pong)
		glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);
		outlineBlurShader->Activate();
		glUniform1i(glGetUniformLocation(outlineBlurShader->ID, "_MainTex"), 0);
		glBindTexture(GL_TEXTURE_2D, outlineRT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glUniform4fv(glGetUniformLocation(outlineBlurShader->ID, "_MainTex_TexelSize"), 1, (float*)&texelSize);
		blurDirection = glm::vec2(0.0f, 1.0f);
		glUniform2fv(glGetUniformLocation(outlineBlurShader->ID, "_BlurDirection"), 1, (float*)&blurDirection);
		glBindVertexArray(rectVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
		glDisable(GL_DEPTH_TEST);

		// merge outline
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		outlineMergeShader->Activate();
		glUniform1i(glGetUniformLocation(outlineMergeShader->ID, "_MainTex"), 0);
		glBindTexture(GL_TEXTURE_2D, postprocessRT);
		glUniform4fv(glGetUniformLocation(outlineMergeShader->ID, "_MainTex_TexelSize"), 1, (float*)&texelSize);
		glm::vec4 outlineColor(1.0f, 0.4f, 0.0f, 0.0f);
		glUniform4fv(glGetUniformLocation(outlineMergeShader->ID, "_OutlineColor"), 1, (float*)&outlineColor);
		glUniform1f(glGetUniformLocation(outlineMergeShader->ID, "_OutlineFade"), 1);
		glBindVertexArray(rectVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	// postprocess
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);
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