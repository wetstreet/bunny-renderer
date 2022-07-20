#include "Skybox.h"
#include "Shader.h"
#include "stb_image.h"
#include <string>

float skyboxVertices[] =
{
	//   Coordinates
	-1.0f, -1.0f,  1.0f,//        7--------6
	 1.0f, -1.0f,  1.0f,//       /|       /|
	 1.0f, -1.0f, -1.0f,//      4--------5 |
	-1.0f, -1.0f, -1.0f,//      | |      | |
	-1.0f,  1.0f,  1.0f,//      | 3------|-2
	 1.0f,  1.0f,  1.0f,//      |/       |/
	 1.0f,  1.0f, -1.0f,//      0--------1
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};
Skybox::Skybox()
{
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
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	// All the faces of the cubemap (make sure they are in this exact order)
	std::string facesCubemap[6] =
	{
		"res/obj/skybox/right.jpg",
		"res/obj/skybox/left.jpg",
		"res/obj/skybox/top.jpg",
		"res/obj/skybox/bottom.jpg",
		"res/obj/skybox/front.jpg",
		"res/obj/skybox/back.jpg"
	};

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

	// Cycles through all the textures and attaches them to the cubemap object
	textures = new unsigned char* [6];
	stbi_set_flip_vertically_on_load(false);
	for (unsigned int i = 0; i < 6; i++)
	{
		textures[i] = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textures[i]);
	}
}

Skybox::~Skybox()
{
	for (unsigned int i = 0; i < 6; i++)
		stbi_image_free(textures[i]);
}

void Skybox::Draw(Camera &camera)
{
	glDisable(GL_CULL_FACE);

    // Since the cubemap will always have a depth of 1.0, we need that equal sign so it doesn't get discarded
    glDepthFunc(GL_LEQUAL);

    Shader::skyboxShader->Activate();
    // We make the mat4 into a mat3 and then a mat4 again in order to get rid of the last row and column
    // The last row and column affect the translation of the skybox (which we don't want to affect)
    glm::mat4 view = glm::mat4(glm::mat3(camera.view));
    glUniformMatrix4fv(glGetUniformLocation(Shader::skyboxShader->ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(Shader::skyboxShader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(camera.projection));

    // Draws the cubemap as the last object so we can save a bit of performance by discarding all fragments
    // where an object is present (a depth of 1.0f will always fail against any object's depth value)
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Switch back to the normal depth function
    glDepthFunc(GL_LESS);
}

/*
 * for cubemap sampling, see subsection 3.7.5 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 */
static int select_cubemap_face(glm::vec3 direction, glm::vec2* texcoord) {
	float abs_x = (float)fabs(direction.x);
	float abs_y = (float)fabs(direction.y);
	float abs_z = (float)fabs(direction.z);
	float ma, sc, tc;
	int face_index;

	if (abs_x > abs_y && abs_x > abs_z) {   /* major axis -> x */
		ma = abs_x;
		if (direction.x > 0) {                  /* positive x */
			face_index = 0;
			sc = -direction.z;
			tc = -direction.y;
		}
		else {                                /* negative x */
			face_index = 1;
			sc = +direction.z;
			tc = -direction.y;
		}
	}
	else if (abs_y > abs_z) {             /* major axis -> y */
		ma = abs_y;
		if (direction.y > 0) {                  /* positive y */
			face_index = 2;
			sc = +direction.x;
			tc = +direction.z;
		}
		else {                                /* negative y */
			face_index = 3;
			sc = +direction.x;
			tc = -direction.z;
		}
	}
	else {                                /* major axis -> z */
		ma = abs_z;
		if (direction.z > 0) {                  /* positive z */
			face_index = 4;
			sc = +direction.x;
			tc = -direction.y;
		}
		else {                                /* negative z */
			face_index = 5;
			sc = -direction.x;
			tc = -direction.y;
		}
	}

	texcoord->x = (sc / ma + 1) / 2;
	texcoord->y = (tc / ma + 1) / 2;
	return face_index;
}

glm::vec4 Skybox::texCube(glm::vec3 direction)
{
	glm::vec2 uv;
	int face_index = select_cubemap_face(glm::normalize(direction), &uv);

	unsigned char* bytes = textures[face_index];

	uv.x = glm::clamp(uv.x, 0.0f, 1.0f);
	uv.y = glm::clamp(uv.y, 0.0f, 1.0f);

	glm::ivec2 intuv(uv.x * width, uv.y * height);
	int index = intuv.x + intuv.y * width;
	return glm::vec4(bytes[index * 3] / 255.0f, bytes[index * 3 + 1] / 255.0f, bytes[index * 3 + 2] / 255.0f, 1);
}