#include "D3DApp.h"

using namespace Framework;
using DX::ThrowIfFailed;

class Chapter4 : public FD3DApp
{
public:
	Chapter4(HINSTANCE hInstance)
		: FD3DApp(hInstance)
	{
	}
	~Chapter4() = default;

    void UpdateScene(float dt) override;
    void DrawScene() override;
};



void Chapter4::UpdateScene(float dt)
{

}

void Chapter4::DrawScene()
{
	assert(DeviceContext);
	assert(SwapChain);

	DeviceContext->ClearRenderTargetView(RenderTargetView.Get(), reinterpret_cast<const float*>(&DirectX::Colors::Blue));
	DeviceContext->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	ThrowIfFailed(SwapChain->Present(0, 0));
}


std::unique_ptr<Framework::FD3DApp> GetApplication(HINSTANCE hInstance)
{
	return std::make_unique<Chapter4>(hInstance);
}