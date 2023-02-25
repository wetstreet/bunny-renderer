#include "Texture.h"
#include "Utils.h"

#include "tinyexr.h"

TexFunc TextureRegisterFunction;
TexFunc TextureUnregisterFunction;

glm::vec2 Texture::vec2_zero = glm::vec2(0, 0); 
glm::vec2 Texture::vec2_one = glm::vec2(1, 1);

std::shared_ptr<Texture> Texture::white_tex;
std::shared_ptr<Texture> Texture::normal_tex;

void Texture::Init()
{
	white_tex = std::make_shared<Texture>("res/obj/white_texture.png");
	normal_tex = std::make_shared<Texture>("res/obj/normal_texture.png");
}

void Texture::Uninit()
{
	white_tex = nullptr;
	normal_tex = nullptr;
}

void FlipYTexture(const unsigned int width, const unsigned int height, float* data) {
	const unsigned int rowsSwapCount = height / 2;
	const unsigned int maxRowIndex = height - 1;

	for (unsigned int i = 0; i < rowsSwapCount; ++i) {
		for (unsigned int j = 0; j < width * 4; ++j) {
			const unsigned int currentDataIndex = width * 4 * i + j;
			const unsigned int swapDataIndex = width * 4 * (maxRowIndex - i) + j;
			std::swap(data[currentDataIndex], data[swapDataIndex]);
		}
	}
}

Texture::Texture(std::string path, GLenum type, GLenum wrap, bool mipmap) : path(path)
{
	name = GetFileNameFromPath(path);
	this->type = type;
	this->wrap = wrap;
	this->mipmap = mipmap;

	std::string suffix = GetSuffix(path);
	if (suffix == "exr")
	{
		const char* err = nullptr;
		int ret = LoadEXR(&data, &width, &height, path.c_str(), &err);
		FlipYTexture(width, height, data);
		if (ret != TINYEXR_SUCCESS)
		{
			fprintf(stderr, "ERR : %s\n", err);
			FreeEXRErrorMessage(err); // release memory of error message.
		}
		numColCh = 4;
	}
	else
	{
		stbi_set_flip_vertically_on_load(true);
		if (type == GL_FLOAT)
			data = stbi_loadf(path.c_str(), &width, &height, &numColCh, 0);
		else
			bytes = stbi_load(path.c_str(), &width, &height, &numColCh, 0);
	}

	TextureRegisterFunction(this);
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

Texture::~Texture()
{
	if (bytes)
		stbi_image_free(bytes);

	if (data)
		stbi_image_free(data);

	TextureUnregisterFunction(this);
}