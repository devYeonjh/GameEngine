#include "App/Renderer.h"

using namespace App;

void Renderer::Create(HWND hWindow, bool useDepth)
{
    CreateDeviceAndSwapChain(hWindow);
    CreateFrameBuffer();
    CreateRasterizerState();

    if (useDepth)
    {
        DXGI_SWAP_CHAIN_DESC desc = {};
        SwapChain->GetDesc(&desc);
        CreateDepthStencilBuffer(desc.BufferDesc.Width, desc.BufferDesc.Height);
    }
}

void Renderer::Release()
{
    // 렌더타겟 해제
    DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ReleaseShader();
    ReleaseConstantBuffer();

    if (DepthStencilView) { DepthStencilView->Release();   DepthStencilView = nullptr; }
    if (DepthStencilBuffer) { DepthStencilBuffer->Release(); DepthStencilBuffer = nullptr; }
    if (RasterizerState) { RasterizerState->Release();    RasterizerState = nullptr; }

    ReleaseFrameBuffer();
    ReleaseDeviceAndSwapChain();
}

void Renderer::OnResize(HWND hWindow)
{
    DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    ReleaseFrameBuffer();

    SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

    CreateFrameBuffer();

    // 뷰포트 갱신
    DXGI_SWAP_CHAIN_DESC desc = {};
    SwapChain->GetDesc(&desc);
    ViewportInfo.Width = (float)desc.BufferDesc.Width;
    ViewportInfo.Height = (float)desc.BufferDesc.Height;

    if (DepthStencilBuffer)
    {
        DepthStencilView->Release();   DepthStencilView = nullptr;
        DepthStencilBuffer->Release(); DepthStencilBuffer = nullptr;
        CreateDepthStencilBuffer(desc.BufferDesc.Width, desc.BufferDesc.Height);
    }
}


void Renderer::CreateShader(const wchar_t* shaderPath)
{
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;

    D3DCompileFromFile(shaderPath, nullptr, nullptr,
        "mainVS", "vs_5_0", 0, 0, &vsBlob, nullptr);
    Device->CreateVertexShader(
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        nullptr, &SimpleVertexShader);

    D3DCompileFromFile(shaderPath, nullptr, nullptr,
        "mainPS", "ps_5_0", 0, 0, &psBlob, nullptr);
    Device->CreatePixelShader(
        psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
        nullptr, &SimplePixelShader);

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    Device->CreateInputLayout(layout, ARRAYSIZE(layout),
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        &SimpleInputLayout);

    Stride = sizeof(FVertexSimple);

    vsBlob->Release();
    psBlob->Release();
}

void Renderer::ReleaseShader()
{
    if (SimpleInputLayout) { SimpleInputLayout->Release();  SimpleInputLayout = nullptr; }
    if (SimplePixelShader) { SimplePixelShader->Release();  SimplePixelShader = nullptr; }
    if (SimpleVertexShader) { SimpleVertexShader->Release(); SimpleVertexShader = nullptr; }
}

void Renderer::CreateConstantBuffer(UINT byteSize)
{
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = (byteSize + 0xf) & 0xfffffff0;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    Device->CreateBuffer(&desc, nullptr, &ConstantBuffer);
}

void Renderer::ReleaseConstantBuffer()
{
    if (ConstantBuffer) { ConstantBuffer->Release(); ConstantBuffer = nullptr; }
}

void Renderer::UpdateConstantBuffer(const void* data, UINT byteSize)
{
    if (!ConstantBuffer) return;

    D3D11_MAPPED_SUBRESOURCE msr = {};
    DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    memcpy(msr.pData, data, byteSize);
    DeviceContext->Unmap(ConstantBuffer, 0);
}

ID3D11Buffer* Renderer::CreateVertexBuffer(const void* vertices, UINT byteWidth)
{
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = byteWidth;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA srd = { vertices };

    ID3D11Buffer* buffer = nullptr;
    Device->CreateBuffer(&desc, &srd, &buffer);
    return buffer;
}

void Renderer::ReleaseVertexBuffer(ID3D11Buffer* buffer)
{
    if (buffer) buffer->Release();
}

void Renderer::BeginFrame()
{
    DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor);
    if (DepthStencilView)
        DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DeviceContext->RSSetViewports(1, &ViewportInfo);
    DeviceContext->RSSetState(RasterizerState);
    DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, DepthStencilView);
    DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void Renderer::PrepareShader()
{
    DeviceContext->VSSetShader(SimpleVertexShader, nullptr, 0);
    DeviceContext->PSSetShader(SimplePixelShader, nullptr, 0);
    DeviceContext->IASetInputLayout(SimpleInputLayout);
    if (ConstantBuffer)
        DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
}

void Renderer::RenderPrimitive(ID3D11Buffer* vertexBuffer, UINT numVertices)
{
    UINT offset = 0;
    DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &Stride, &offset);
    DeviceContext->Draw(numVertices, 0);
}

void Renderer::EndFrame()
{
    SwapChain->Present(1, 0);
}

void Renderer::CreateDeviceAndSwapChain(HWND hWindow)
{
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

    DXGI_SWAP_CHAIN_DESC desc = {};
    desc.BufferDesc.Width = 0;
    desc.BufferDesc.Height = 0;
    desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.OutputWindow = hWindow;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
        featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
        &desc, &SwapChain, &Device, nullptr, &DeviceContext);

    SwapChain->GetDesc(&desc);
    ViewportInfo = { 0.0f, 0.0f,
        (float)desc.BufferDesc.Width, (float)desc.BufferDesc.Height,
        0.0f, 1.0f };
}

void Renderer::CreateFrameBuffer()
{
    SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    Device->CreateRenderTargetView(FrameBuffer, &rtvDesc, &FrameBufferRTV);
}

void Renderer::CreateDepthStencilBuffer(UINT width, UINT height)
{
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    Device->CreateTexture2D(&texDesc, nullptr, &DepthStencilBuffer);
    Device->CreateDepthStencilView(DepthStencilBuffer, nullptr, &DepthStencilView);
}

void Renderer::CreateRasterizerState()
{
    D3D11_RASTERIZER_DESC desc = {};
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_BACK;

    Device->CreateRasterizerState(&desc, &RasterizerState);
}

void Renderer::ReleaseDeviceAndSwapChain()
{
    if (DeviceContext) { DeviceContext->Flush(); }
    if (SwapChain) { SwapChain->Release();     SwapChain = nullptr; }
    if (Device) { Device->Release();        Device = nullptr; }
    if (DeviceContext) { DeviceContext->Release(); DeviceContext = nullptr; }
}

void Renderer::ReleaseFrameBuffer()
{
    if (FrameBuffer) { FrameBuffer->Release();    FrameBuffer = nullptr; }
    if (FrameBufferRTV) { FrameBufferRTV->Release(); FrameBufferRTV = nullptr; }
}