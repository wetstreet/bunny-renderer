#include "Texture.h"
#include "Utils.h"

Texture::Texture(const char* image)
	: path(image)
{
	name = GetFileNameFromPath(path);

	stbi_set_flip_vertically_on_load(true);
	bytes = stbi_load(image, &width, &height, &numColCh, 0);
	//std::cout << "Load texture "<< name <<", channl count is " << numColCh << std::endl;

	GLenum format = GL_RGB;
	if (numColCh == 4)
		format = GL_RGBA;

	glGenTextures(1, &ID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ID);

	glObjectLabel(GL_TEXTURE, ID, -1, GetFileNameFromPath(path).c_str());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, bytes);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit)
{
	GLuint texUni = glGetUniformLocation(shader.ID, uniform);
	shader.Activate();
	glUniform1i(texUni, unit);
}

glm::vec4 Texture::tex2D(glm::vec2& uv)
{
	glm::ivec2 intuv(uv.x * width, uv.y * height);
	int index = intuv.x + intuv.y * width;
	return glm::vec4(bytes[index * numColCh] / 255.0f, bytes[index * numColCh + 1] / 255.0f, bytes[index * numColCh + 2] / 255.0f,
		numColCh == 4 ? bytes[index * numColCh + 3] / 255.0f : 1);
}

void Texture::Bind()
{
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
	stbi_image_free(bytes);
	glDeleteTextures(1, &ID);
}