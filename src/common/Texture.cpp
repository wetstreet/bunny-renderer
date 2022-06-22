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
	glBindTexture(GL_TEXTURE_2D, ID);

	glObjectLabel(GL_TEXTURE, ID, -1, GetFileNameFromPath(path).c_str());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, bytes);
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
	// clamp
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
	stbi_image_free(bytes);
	glDeleteTextures(1, &ID);
}