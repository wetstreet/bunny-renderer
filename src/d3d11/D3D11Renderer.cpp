#include "D3D11Renderer.h"
#include "common/Selection.h"

#include "common/Utils.h"

D3D11Renderer::D3D11Renderer()
{
}

bool D3D11Renderer::Init(HWND hwnd)
{
    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        return false;
    }
    return true;
}

// Helper functions
bool D3D11Renderer::CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain, &pd3dDevice, &featureLevel, &pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain, &pd3dDevice, &featureLevel, &pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateBackBuffer();
    CreateRenderTarget();
    return true;
}

void D3D11Renderer::RegisterMesh(Mesh* mesh)
{
    // Create Vertex and Index Buffer
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    // UINT numVerts;
    UINT numIndices;
    UINT stride;
    UINT offset;
    {
        stride = sizeof(Vertex);
        // numVerts = obj.numVertices;
        offset = 0;
        numIndices = mesh->indices.size();

        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.ByteWidth = mesh->vertices.size() * sizeof(Vertex);
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexSubresourceData = { &mesh->vertices[0] };

        HRESULT hResult = pd3dDevice->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &vertexBuffer);
        assert(SUCCEEDED(hResult));

        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.ByteWidth = mesh->indices.size() * sizeof(uint32_t);
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexSubresourceData = { &mesh->indices[0] };

        hResult = pd3dDevice->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &indexBuffer);
        assert(SUCCEEDED(hResult));
    }

    std::shared_ptr<D3D11Mesh> d3d11mesh = std::make_shared<D3D11Mesh>();
    d3d11mesh->vertexBuffer = vertexBuffer;
    d3d11mesh->indexBuffer = indexBuffer;
    d3d11mesh->numIndices = numIndices;
    d3d11mesh->stride = stride;
    d3d11mesh->offset = offset;
    meshDict[mesh] = d3d11mesh;
}

void D3D11Renderer::UnregisterMesh(Mesh* mesh)
{
    std::shared_ptr<D3D11Mesh> d3d11mesh = meshDict[mesh];
    d3d11mesh->vertexBuffer->Release();
    d3d11mesh->indexBuffer->Release();
    meshDict.erase(mesh);
}

void D3D11Renderer::RegisterTexture(Texture* tex)
{
    if (tex->bytes == nullptr)
        return;
    ID3D11ShaderResourceView* textureView;
    {
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = tex->width;
        textureDesc.Height = tex->height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
        textureSubresourceData.pSysMem = tex->bytes;
        textureSubresourceData.SysMemPitch = 4 * tex->width;

        ID3D11Texture2D* texture;
        pd3dDevice->CreateTexture2D(&textureDesc, &textureSubresourceData, &texture);

        pd3dDevice->CreateShaderResourceView(texture, nullptr, &textureView);
        texture->Release();
    }

    std::shared_ptr<TextureData> texData = std::make_shared<TextureData>();
    texData->srv = textureView;
    texDict[tex] = texData;
}

void D3D11Renderer::UnregisterTexture(Texture* texture)
{
    if (texture->bytes == nullptr)
        return;
    std::shared_ptr<TextureData> texData = texDict[texture];
    texData->srv->Release();
    texDict.erase(texture);
}

void D3D11Renderer::BindTexture(Texture& texture, GLuint slot)
{
    if (texture.bytes == nullptr)
        return;
    std::shared_ptr<TextureData> texdata = texDict[&texture];
    pd3dDeviceContext->PSSetShaderResources(0, 1, &texdata->srv);
    pd3dDeviceContext->PSSetSamplers(0, 1, &samplerState);
}

void D3D11Renderer::CreateBackBuffer()
{
    ID3D11Texture2D* pBackBuffer;
    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
    pBackBuffer->Release();
}

void D3D11Renderer::UpdateBackBuffer(int width, int height)
{
    pd3dDeviceContext->OMSetRenderTargets(0, 0, 0);
    mainRenderTargetView->Release();

    HRESULT res = pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    assert(SUCCEEDED(res));

    ID3D11Texture2D* d3d11FrameBuffer;
    res = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer);
    assert(SUCCEEDED(res));

    res = pd3dDevice->CreateRenderTargetView(d3d11FrameBuffer, NULL, &mainRenderTargetView);
    assert(SUCCEEDED(res));
    d3d11FrameBuffer->Release();
}

void D3D11Renderer::CreateRenderTarget()
{
    D3D11_TEXTURE2D_DESC textureDesc;
    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

    ///////////////////////// Map's Texture
    // Initialize the  texture description.
    ZeroMemory(&textureDesc, sizeof(textureDesc));

    // Setup the texture description.
    // We will have our map be a square
    // We will need to have this texture bound as a render target AND a shader resource
    textureDesc.Width = viewport.x;
    textureDesc.Height = viewport.y;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    // Create the texture
    ID3D11Texture2D* renderTargetTextureMap = NULL;
    pd3dDevice->CreateTexture2D(&textureDesc, NULL, &renderTargetTextureMap);
    rtSize = viewport;

    /////////////////////// Map's Render Target
    // Setup the description of the render target view.
    renderTargetViewDesc.Format = textureDesc.Format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    renderTargetViewDesc.Texture2D.MipSlice = 0;

    // Create the render target view.
    pd3dDevice->CreateRenderTargetView(renderTargetTextureMap, &renderTargetViewDesc, &renderTargetViewMap);

    /////////////////////// Map's Shader Resource View
    // Setup the description of the shader resource view.
    shaderResourceViewDesc.Format = textureDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    // Create the shader resource view.
    pd3dDevice->CreateShaderResourceView(renderTargetTextureMap, &shaderResourceViewDesc, &renderTargetSRV);

    D3D11_TEXTURE2D_DESC depthBufferDesc;
    renderTargetTextureMap->GetDesc(&depthBufferDesc);

    renderTargetTextureMap->Release();

    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthBuffer;
    pd3dDevice->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);

    pd3dDevice->CreateDepthStencilView(depthBuffer, nullptr, &depthBufferView);

    depthBuffer->Release();
}

void D3D11Renderer::CleanupRenderTarget()
{
    if (mainRenderTargetView) { mainRenderTargetView->Release(); mainRenderTargetView = NULL; }
}

void D3D11Renderer::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (pSwapChain) { pSwapChain->Release(); pSwapChain = NULL; }
    if (pd3dDeviceContext) { pd3dDeviceContext->Release(); pd3dDeviceContext = NULL; }
    if (pd3dDevice) { pd3dDevice->Release(); pd3dDevice = NULL; }
}

bool D3D11Renderer::InitRender()
{
    // Create Vertex Shader
    ID3DBlob* vsBlob;
    {
        ID3DBlob* shaderCompileErrorsBlob;
        HRESULT hResult = D3DCompileFromFile(L"./res/shaders/hlsl/shaders.hlsl", nullptr, nullptr, "vs_main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsBlob, &shaderCompileErrorsBlob);
        if (FAILED(hResult))
        {
            const char* errorString = NULL;
            if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                errorString = "Could not compile shader; file not found";
            else if (shaderCompileErrorsBlob) {
                errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
                shaderCompileErrorsBlob->Release();
            }
            MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
            return false;
        }

        hResult = pd3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
        assert(SUCCEEDED(hResult));
    }

    // Create Pixel Shader
    {
        ID3DBlob* psBlob;
        ID3DBlob* shaderCompileErrorsBlob;
        HRESULT hResult = D3DCompileFromFile(L"./res/shaders/hlsl/shaders.hlsl", nullptr, nullptr, "ps_main", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &psBlob, &shaderCompileErrorsBlob);
        if (FAILED(hResult))
        {
            const char* errorString = NULL;
            if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                errorString = "Could not compile shader; file not found";
            else if (shaderCompileErrorsBlob) {
                errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
                shaderCompileErrorsBlob->Release();
            }
            MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
            return false;
        }

        hResult = pd3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
        assert(SUCCEEDED(hResult));
        psBlob->Release();
    }

    // Create Input Layout
    {
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
        {
            { "POS",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                               D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEX",    0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TANGENT",0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        HRESULT hResult = pd3dDevice->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
        assert(SUCCEEDED(hResult));
        vsBlob->Release();
    }

    // Create Constant Buffer
    {
        D3D11_BUFFER_DESC constantBufferDesc = {};
        // ByteWidth must be a multiple of 16, per the docs
        constantBufferDesc.ByteWidth = sizeof(VSConstants) + 0xf & 0xfffffff0;
        constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT hResult = pd3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &VSConstantBuffer);
        assert(SUCCEEDED(hResult));
    }

    {
        D3D11_BUFFER_DESC constantBufferDesc = {};
        // ByteWidth must be a multiple of 16, per the docs
        constantBufferDesc.ByteWidth = sizeof(PSConstants) + 0xf & 0xfffffff0;
        constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT hResult = pd3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &PSConstantBuffer);
        assert(SUCCEEDED(hResult));
    }

    {
        D3D11_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_BACK;
        rasterizerDesc.FrontCounterClockwise = TRUE;

        pd3dDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
    }

    {
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable = TRUE;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

        pd3dDevice->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
    }

    // Create Sampler State
    {
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.BorderColor[0] = 1.0f;
        samplerDesc.BorderColor[1] = 1.0f;
        samplerDesc.BorderColor[2] = 1.0f;
        samplerDesc.BorderColor[3] = 1.0f;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

        pd3dDevice->CreateSamplerState(&samplerDesc, &samplerState);
    }

    return true;
}

D3D11Renderer::~D3D11Renderer()
{
}

int D3D11Renderer::GetObjectID(int x, int y)
{
	return 0;
}

void D3D11Renderer::DrawMesh(Mesh& mesh)
{
    std::shared_ptr<D3D11Mesh> d3d11mesh = meshDict[&mesh];

    pd3dDeviceContext->IASetVertexBuffers(0, 1, &d3d11mesh->vertexBuffer, &d3d11mesh->stride, &d3d11mesh->offset);
    pd3dDeviceContext->IASetIndexBuffer(d3d11mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    pd3dDeviceContext->DrawIndexed(d3d11mesh->numIndices, 0, 0);
}

void D3D11Renderer::Render(Scene& scene)
{
	scene.UpdateMatrices();
    scene.camera.updateMatrix(viewport.x, viewport.y);

    if (rtSize != viewport)
    {
        pd3dDeviceContext->OMSetRenderTargets(0, 0, 0);
        renderTargetViewMap->Release();
        renderTargetSRV->Release();
        depthBufferView->Release();

        CreateRenderTarget();
    }

    FLOAT backgroundColor[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
    pd3dDeviceContext->ClearRenderTargetView(renderTargetViewMap, backgroundColor);

    pd3dDeviceContext->ClearDepthStencilView(depthBufferView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    D3D11_VIEWPORT d3d11_viewport = { 0.0f, 0.0f, (FLOAT)viewport.x, (FLOAT)viewport.y, 0.0f, 1.0f };
    pd3dDeviceContext->RSSetViewports(1, &d3d11_viewport);

    pd3dDeviceContext->RSSetState(rasterizerState);
    pd3dDeviceContext->OMSetDepthStencilState(depthStencilState, 0);

    pd3dDeviceContext->OMSetRenderTargets(1, &renderTargetViewMap, depthBufferView);

    pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pd3dDeviceContext->IASetInputLayout(inputLayout);

    glm::vec3 lightPos;
    glm::vec3 lightColor;
    scene.GetMainLightProperties(lightPos, lightColor);

    // render scene
    for (int i = 0; i < scene.objects.size(); i++)
    {
        std::shared_ptr<Object> object = scene.objects[i];
        if (object->GetType() == Type_Mesh)
        {
            std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(object);
            if (mesh->isEnabled)
            {

                pd3dDeviceContext->VSSetShader(vertexShader, nullptr, 0);
                pd3dDeviceContext->PSSetShader(pixelShader, nullptr, 0);

                pd3dDeviceContext->VSSetConstantBuffers(0, 1, &VSConstantBuffer);
                pd3dDeviceContext->PSSetConstantBuffers(0, 1, &PSConstantBuffer);

                // Update constant buffer
                {
                    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
                    pd3dDeviceContext->Map(VSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
                    VSConstants* constants = (VSConstants*)(mappedSubresource.pData);
                    constants->br_ObjectToClip = glm::transpose(scene.camera.cameraMatrix * mesh->objectToWorld);
                    constants->br_ObjectToWorld = glm::transpose(mesh->objectToWorld);
                    constants->br_WorldToObject = glm::transpose(mesh->worldToObject);
                    pd3dDeviceContext->Unmap(VSConstantBuffer, 0);
                }

                {
                    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
                    pd3dDeviceContext->Map(PSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
                    PSConstants* constants = (PSConstants*)(mappedSubresource.pData);
                    constants->_MainLightPosition = glm::vec4(lightPos, 0);
                    constants->_MainLightColor = glm::vec4(lightColor, 1);
                    pd3dDeviceContext->Unmap(PSConstantBuffer, 0);
                }

                BindTexture(*Texture::brick_tex, 0);


                DrawMesh(*mesh);
            }
        }
    }

}