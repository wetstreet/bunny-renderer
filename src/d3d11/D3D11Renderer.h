#ifndef __D3D11_RENDERER_H__
#define __D3D11_RENDERER_H__

#include "common/RealtimeRenderer.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") // Automatically link with d3dcompiler.lib as we are using D3DCompile() below.
#endif
#include <tchar.h>

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
    void CreateRenderTarget();
    void CleanupRenderTarget();

    //virtual void GenerateCubemapFromEquirectangular(Scene& scene);
    //virtual void GenerateIrradianceMap(Scene& scene);
    //virtual void GeneratePrefilterMap(Scene& scene);
    //virtual void GenerateBrdfLUT(Scene& scene);

    void SetRenderTarget(const float* clearColor)
    {
        pd3dDeviceContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
        pd3dDeviceContext->ClearRenderTargetView(mainRenderTargetView, clearColor);
    }

    void Present()
    {
        pSwapChain->Present(0, 0); // Present without vsync
    }

    virtual void RegisterTexture(Texture* texture);

    virtual void BindTexture(Texture& texture);

    virtual void* GetRT() { return (void*)shaderResourceViewMap; };
public:
    GLuint postprocessRT;
    GLuint outlineRT;

    ID3D11Device* pd3dDevice = NULL;
    ID3D11DeviceContext* pd3dDeviceContext = NULL;
    IDXGISwapChain* pSwapChain = NULL;
    ID3D11RenderTargetView* mainRenderTargetView = NULL;

    ID3D11Texture2D* renderTargetTextureMap = NULL;
    ID3D11RenderTargetView* renderTargetViewMap = NULL;
    ID3D11ShaderResourceView* shaderResourceViewMap = NULL;


private:
    const unsigned int width = 1400;
    const unsigned int height = 900;

    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11InputLayout* inputLayout;
    ID3D11Buffer* vertexBuffer;
    UINT numVerts;
    UINT stride;
    UINT offset;

    GLuint objectIdRT;
    GLuint rectVAO, rectVBO;

    GLuint rbo;
    GLuint FBO;

    GLuint postprocessFBO;
    GLuint outlineFBO;

    GLuint captureFBO, captureRBO;
    GLuint envCubemap;
    GLuint irradianceMap;
    GLuint prefilterMap;
    GLuint brdfLUTTexture;

    unsigned int shadowMapWidth = 2048, shadowMapHeight = 2048;

    GLuint shadowMap;
    GLuint shadowMapFBO;

    std::shared_ptr<Shader> equirectangularShader;
    std::shared_ptr<Shader> irradianceShader;
    std::shared_ptr<Shader> prefilterShader;

    std::shared_ptr<Shader> shadowMapShader;

    std::shared_ptr<Shader> postprocessShader;

    std::shared_ptr<Shader> outlineShader;
    std::shared_ptr<Shader> outlineCompareShader;
    std::shared_ptr<Shader> outlineBlurShader;
    std::shared_ptr<Shader> outlineMergeShader;
};

#endif //__D3D11_RENDERER_H__