#include "D3D11Renderer.h"
#include "common/Selection.h"

D3D11Renderer::D3D11Renderer()
{
}

D3D11Renderer::~D3D11Renderer()
{
}

void D3D11Renderer::RegisterTexture(unsigned int ID, const char* name, int width, int height, GLenum format, GLenum type, GLenum wrap, bool mipmap, unsigned char* bytes)
{
}

void D3D11Renderer::RegisterTextureF(unsigned int ID, const char* name, int width, int height, GLenum format, GLenum type, GLenum wrap, bool mipmap, float* data)
{
}

void D3D11Renderer::BindTexture(Texture& texture)
{
}

int D3D11Renderer::GetObjectID(int x, int y)
{
	return 0;
}

void D3D11Renderer::Render(Scene& scene)
{
	scene.UpdateMatrices();

}