#include "D3DApp.h"
#include <sstream>
#include "DebugLog.h"

using namespace Framework;
using Debug::DebugLog;
using DX::ThrowIfFailed;
using Microsoft::WRL::ComPtr;

class Chapter4 : public FD3DApp
{
public:
	Chapter4(HINSTANCE hInstance)
		: FD3DApp(hInstance)
	{
	}
	~Chapter4() = default;

	bool Init() override;
    void UpdateScene(float dt) override;
    void DrawScene() override;
};

bool Chapter4::Init()
{
	FD3DApp::Init();

	// 디바이스를 만들 때 사용한 Factory 호출
	ComPtr<IDXGIDevice> dxgiDevice;
	ThrowIfFailed(Device->QueryInterface(__uuidof(IDXGIDevice), (void**)dxgiDevice.GetAddressOf()));

	ComPtr<IDXGIAdapter> dxgiAdapter;
	ThrowIfFailed(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)dxgiAdapter.GetAddressOf()));

	ComPtr<IDXGIFactory> dxgiFactory;
	ThrowIfFailed(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)dxgiFactory.GetAddressOf()));

	// Exercise 1. Alt+Enter 기능 비활성화
	ThrowIfFailed(dxgiFactory->MakeWindowAssociation(AppWnd, DXGI_MWA_NO_WINDOW_CHANGES));

	// Exercise 2 ~ 5 OutputDebugString을 사용한 출력. (Wrapper 함수 DebugLog 사용)
	// https://learn.microsoft.com/ko-kr/windows/win32/api/dxgi/nf-dxgi-idxgifactory-enumadapters
	// 어댑터 열거 예제 참고
	UINT AdapterNum = 0;							// Adapter 개수 저장
	ComPtr<IDXGIAdapter> pAdapter;					// vAdapters에 저장하기 위한 임시 포인터
	std::vector<ComPtr<IDXGIAdapter>> vAdapters;	// 현재 시스템에 존재하는 Adapter 배열 
	while (dxgiFactory->EnumAdapters(AdapterNum, pAdapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND)
	{
		vAdapters.push_back(pAdapter);
		++AdapterNum;
	}

	DebugLog(L"EX2) 어댑터 개수 = %d\n", AdapterNum);

	// IDXGIAdapter::CheckInterfaceSupport 함수는 DX11을 지원하지 않음 https://learn.microsoft.com/ko-kr/windows/win32/api/dxgi/nf-dxgi-idxgiadapter-checkinterfacesupport
	// 따라서 D3D11CreateDevice 함수로 확인
	for (int i = 0; i < AdapterNum; ++i)
	{
		HRESULT hr = D3D11CreateDevice(
			vAdapters[i].Get(),
			D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			0,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			nullptr,
			nullptr,
			nullptr);

		if (SUCCEEDED(hr))
		{
			DebugLog(L"EX3) 어댑터 %d D3D11 지원함.\n", i);
		}
		else
		{
			DebugLog(L"EX3) 어댑터 %d D3D11 지원하지않음.\n", i);
		}
	}

	// https://learn.microsoft.com/ko-kr/windows/win32/api/dxgi/nf-dxgi-idxgiadapter-enumoutputs
	// Output 열거 예제 참고
	UINT OutputNum = 0;
	ComPtr<IDXGIOutput> pOutput;
	std::vector<ComPtr<IDXGIOutput>> vOutputs;
	while (dxgiAdapter->EnumOutputs(OutputNum, pOutput.GetAddressOf()) != DXGI_ERROR_NOT_FOUND)
	{
		vOutputs.push_back(pOutput);
		++OutputNum;
	}
	DebugLog(L"EX4) Output 개수 = %d\n", OutputNum);

	// https://learn.microsoft.com/ko-kr/windows/win32/api/dxgi/nf-dxgi-idxgioutput-getdisplaymodelist
	// GetDisplayModeList 설명 참고
	UINT ModeNum = 0;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	UINT flags = DXGI_ENUM_MODES_INTERLACED;

	vOutputs[0]->GetDisplayModeList(format, flags, &ModeNum, 0);
	std::unique_ptr<DXGI_MODE_DESC[]> pDescs = std::make_unique<DXGI_MODE_DESC[]>(ModeNum);
	vOutputs[0].Get()->GetDisplayModeList(format, flags, &ModeNum, pDescs.get());

	for (int i = 0; i < ModeNum; ++i)
	{
		DebugLog(L"EX5) Width = %d, Height = %d, Refresh = %d/%d\n",
			pDescs[i].Width, pDescs[i].Height,
			pDescs[i].RefreshRate.Numerator /*분자*/,
			pDescs[i].RefreshRate.Denominator /*분모*/);
	}

	return true;
}

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