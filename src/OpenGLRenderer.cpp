#include "OpenGLRenderer.h"
#include "opengl/Selection.h"

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
	shader = new Shader("res/shaders/default.vert", "res/shaders/default.frag");
	postprocessShader = new Shader("res/shaders/postprocess.vert", "res/shaders/postprocess.frag");
	outlineShader = new Shader("res/shaders/outline.vert", "res/shaders/outline.frag");
	outlineCompareShader = new Shader("res/shaders/outlineCompareIds.vert", "res/shaders/outlineCompareIds.frag");
	outlineBlurShader = new Shader("res/shaders/outlineBlur.vert", "res/shaders/outlineBlur.frag");
	outlineMergeShader = new Shader("res/shaders/outlineMerge.vert", "res/shaders/outlineMerge.frag");

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

    // depth & stencil rbo
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, viewport.x, viewport.y);  
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);


	// outline fbo
	glGenFramebuffers(1, &outlineFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, outlineFBO);
	// outlinert
	glGenTextures(1, &outlineRT);
	char* name = "outlineRT";
	glBindTexture(GL_TEXTURE_2D, outlineRT);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewport.x, viewport.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postprocessRT, 0);
}

OpenGLRenderer::~OpenGLRenderer()
{
    delete shader;
    delete postprocessShader;
	delete outlineShader;
	delete outlineCompareShader;
	delete outlineBlurShader;
	delete outlineMergeShader;
}

void OpenGLRenderer::Render(Scene &scene)
{
	// update rt size
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport.x, viewport.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, viewport.x, viewport.y);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glViewport(0, 0, viewport.x, viewport.y);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

    glm::vec3 clearColor = scene.camera.clearColor;
    glClearColor(clearColor.x, clearColor.y, clearColor.z, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene.camera.updateMatrix(45.0f, 0.1f, 1000.0f, viewport.x, viewport.y);

    scene.Draw(shader);

    skybox.Draw(scene.camera);

    // post process
    glBindTexture(GL_TEXTURE_2D, postprocessRT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport.x, viewport.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);
    glDisable(GL_DEPTH_TEST);

    postprocessShader->Activate();
	glUniform1i(glGetUniformLocation(postprocessShader->ID, "screenTexture"), 0);
    glBindTexture(GL_TEXTURE_2D, renderTexture);

    glBindVertexArray(rectVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

	if (node_clicked != -1)
	{

		glDepthMask(GL_FALSE);

		// outline
		glBindTexture(GL_TEXTURE_2D, outlineRT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport.x, viewport.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

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
			glUniformMatrix4fv(glGetUniformLocation(outlineShader->ID, "camMatrix"), 1, GL_FALSE, glm::value_ptr(scene.camera.cameraMatrix * mesh->objectToWorld));

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
			glUniformMatrix4fv(glGetUniformLocation(outlineShader->ID, "camMatrix"), 1, GL_FALSE, glm::value_ptr(scene.camera.cameraMatrix * mesh->objectToWorld));

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
		glUniform4fv(glGetUniformLocation(outlineBlurShader->ID, "_MainTex_TexelSize"), 1, (float*)&texelSize);
		blurDirection = glm::vec2(0.0f, 1.0f);
		glUniform2fv(glGetUniformLocation(outlineBlurShader->ID, "_BlurDirection"), 1, (float*)&blurDirection);
		glBindVertexArray(rectVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
		glDisable(GL_DEPTH_TEST);

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

    // editor background
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.22f, 0.22f, 0.22f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}