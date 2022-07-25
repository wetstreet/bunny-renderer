#include "Texture.h"
#include "Utils.h"

glm::vec2 Texture::vec2_zero = glm::vec2(0, 0); 
glm::vec2 Texture::vec2_one = glm::vec2(1, 1);

std::shared_ptr<Texture> Texture::white_tex;
std::shared_ptr<Texture> Texture::normal_tex;

void Texture::Init()
{
	white_tex = std::make_shared<Texture>("res/obj/white_texture.png");
	normal_tex = std::make_shared<Texture>("res/obj/normal_texture.png");
}

Texture::Texture(std::string path, GLenum type, GLenum wrap, bool mipmap) : path(path)
{
	name = GetFileNameFromPath(path);

	stbi_set_flip_vertically_on_load(true);
	if (type == GL_FLOAT)
		data = stbi_loadf(path.c_str(), &width, &height, &numColCh, 0);
	else
		bytes = stbi_load(path.c_str(), &width, &height, &numColCh, 0);
	//std::cout << "Load texture "<< name <<", channl count is " << numColCh << std::endl;

	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);

	glObjectLabel(GL_TEXTURE, ID, -1, GetFileNameFromPath(path).c_str());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

	GLenum format = numColCh == 4 ? GL_RGBA : GL_RGB;

	if (type == GL_FLOAT)
		glTexImage2D(GL_TEXTURE_2D, 0, numColCh == 4 ? GL_RGBA16F : GL_RGB16F, width, height, 0, format, type, data);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, numColCh == 4 ? GL_RGBA8 : GL_RGB8, width, height, 0, format, type, bytes);

	if (mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit)
{
	shader.Activate();
	glUniform1i(glGetUniformLocation(shader.ID, uniform), unit);
}

glm::vec4 Texture::tex2D(glm::vec2& uv)
{
	// todo: clamp or repeat
	glm::vec2 uvModified = glm::clamp(uv, vec2_zero, vec2_one);

	glm::ivec2 intuv(uvModified.x * width, uvModified.y * height);
	int index = intuv.x + intuv.y * width;
	return glm::vec4(bytes[index * numColCh] / 255.0f, bytes[index * numColCh + 1] / 255.0f, bytes[index * numColCh + 2] / 255.0f,
		numColCh == 4 ? bytes[index * numColCh + 3] / 255.0f : 1);
}

void Texture::Bind(GLenum slot)
{
	glActiveTexture(slot);
    glBindTexture(GL_TEXTURE_2D, ID);
}

Texture::~Texture()
{
	if (bytes)
		stbi_image_free(bytes);

	if (data)
		stbi_image_free(data);

	glDeleteTextures(1, &ID);
}