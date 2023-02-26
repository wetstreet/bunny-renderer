#ifndef __D3D11_RENDERER_H__
#define __D3D11_RENDERER_H__

#include "common/RealtimeRenderer.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") // Automatically link with d3dcompiler.lib as we are using D3DCompile() below.
#endif
#include <tchar.h>


struct Constants
{
    glm::mat4 modelViewProj;
};

class D3D11Mesh
{
public:
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    // UINT numVerts;
    UINT numIndices;
    UINT stride;
    UINT offset;
};

class D3D11Renderer : public RealtimeRenderer
{
public:
    D3D11Renderer();
    virtual ~D3D11Renderer();

    virtual int GetObjectID(int x, int y);
    virtual void Render(Scene& scene);

    bool Init(HWND hwnd);
    bool InitRender();

    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();

    void CreateBackBuffer();

    void CreateRenderTarget();
    void CleanupRenderTarget();

    virtual void GenerateCubemapFromEquirectangular(Scene& scene) {};
    virtual void GenerateIrradianceMap(Scene& scene) {};
    virtual void GeneratePrefilterMap(Scene& scene) {};
    virtual void GenerateBrdfLUT(Scene& scene) {};

    virtual void RegisterTexture(Texture* texture);
    virtual void RegisterMesh(Mesh* mesh);
    //virtual void RegisterSkybox(Skybox* skybox);

    //virtual void UnregisterTexture(Texture* texture);
    virtual void UnregisterMesh(Mesh* mesh);
    //virtual void UnregisterSkybox(Skybox* skybox);

    //virtual void BindSkybox(Skybox* skybox, GLuint slot);
    virtual void BindTexture(Texture& texture, GLuint slot);

    //virtual void DrawSkybox();
    //virtual void DrawScene(Scene& scene);
    virtual void DrawMesh(Mesh& mesh);

    void SetRenderTarget(const float* clearColor)
    {
        pd3dDeviceContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
        pd3dDeviceContext->ClearRenderTargetView(mainRenderTargetView, clearColor);
    }

    void Present()
    {
        pSwapChain->Present(0, 0); // Present without vsync
    }

    virtual void* GetRT() { return (void*)renderTargetSRV; };
public:
    GLuint postprocessRT;
    GLuint outlineRT;

    ID3D11Device* pd3dDevice = NULL;
    ID3D11DeviceContext* pd3dDeviceContext = NULL;
    IDXGISwapChain* pSwapChain = NULL;
    ID3D11RenderTargetView* mainRenderTargetView = NULL;

    ID3D11DepthStencilView* depthBufferView = NULL;
    ID3D11RenderTargetView* renderTargetViewMap = NULL;
    ID3D11ShaderResourceView* renderTargetSRV = NULL;

private:
    const unsigned int width = 1400;
    const unsigned int height = 900;

    glm::ivec2 rtSize;

    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;

    std::unordered_map<Mesh*, std::shared_ptr<D3D11Mesh>> meshDict;

    ID3D11InputLayout* inputLayout;
    ID3D11Buffer* constantBuffer;

    ID3D11RasterizerState* rasterizerState;
    ID3D11DepthStencilState* depthStencilState;
};

#endif //__D3D11_RENDERER_H__