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

	glEnable(GL_DEPTH_TEST);

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

    // framebuffer
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

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

	GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(2, buffers);

	// outline fbo
	glGenFramebuffers(1, &outlineFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, outlineFBO);
	// outlinert
	glGenTextures(1, &outlineRT);
	glBindTexture(GL_TEXTURE_2D, outlineRT);
	glObjectLabel(GL_TEXTURE, outlineRT, -1, "Outline RT");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewport.x, viewport.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outlineRT, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // postprocess fbo
	glGenFramebuffers(1, &postprocessFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);

	// postprocess color rt
	glGenTextures(1, &postprocessRT);
	glBindTexture(GL_TEXTURE_2D, postprocessRT);
	glObjectLabel(GL_TEXTURE, postprocessRT, -1, "PostProcess RT");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewport.x, viewport.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postprocessRT, 0);

	// capture fbo
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
}

OpenGLRenderer::~OpenGLRenderer()
{
}

void OpenGLRenderer::GenerateIrradianceMap(Scene& scene)
{
	unsigned int irradianceMap;
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0,
			GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

	Shader shader("editor/ibl");
	shader.Activate();
	shader.SetUniform("environmentMap", 0);
	shader.SetUniform("projection", captureProjection);

	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		shader.SetUniform("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		scene.skybox.DrawMesh();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	scene.skybox.irradianceMap = irradianceMap;
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

	for (int i = 0; i < scene.objects.size(); i++)
	{
		std::shared_ptr<Object> object = scene.objects[i];
		if (object->GetType() == Type_Mesh)
		{
			std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(object);
			if (mesh->isEnabled)
			{
				glUniformMatrix4fv(glGetUniformLocation(shadowMapShader->ID, "br_ObjectToWorld"), 1, GL_FALSE, glm::value_ptr(mesh->objectToWorld));

				mesh->Draw();
			}
		}
	}

	scene.shadowMap = shadowMap;

	// update rt size
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport.x, viewport.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D, objectIdRT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, viewport.x, viewport.y, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D, postprocessRT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport.x, viewport.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
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

    scene.Draw();

    scene.skybox.Draw(scene.camera);

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

			mesh->Draw();
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

			mesh->Draw();
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