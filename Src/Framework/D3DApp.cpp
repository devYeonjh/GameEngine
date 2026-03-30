#include "D3DApp.h"
#include <WindowsX.h>
#include <sstream>
#include "DebugLog.h"

using namespace Framework;
using Debug::DebugLog;
using DX::ThrowIfFailed;
using Microsoft::WRL::ComPtr;

// MainWndProc 전방 선언 Main.cpp에서 정의 됨.
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

FD3DApp::FD3DApp(HINSTANCE hInstance)
	: AppInstance(hInstance),
	MainWndCaption(L"D3D11 Application"),
	DriverType(D3D_DRIVER_TYPE_HARDWARE),
	CanvasWidth(800),
	CanvasHeight(600),
	bIs4xMsaaEnabled(false),
	AppWnd(nullptr),
	bIsAppPaused(false),
	bIsMinimized(false),
	bIsMaximized(false),
	bResizing(false),
	Quality4xMsaa(0)
{
	Viewport = {};
}

HINSTANCE FD3DApp::GetAppInstance() const
{
	return AppInstance;
}

HWND FD3DApp::GetAppWnd() const
{
	return AppWnd;
}

float FD3DApp::GetAspectRatio() const
{
	return static_cast<float>(CanvasWidth) / CanvasHeight;
}

int FD3DApp::Run()
{
	MSG msg = { 0 };

	Timer.Reset();

	while (msg.message != WM_QUIT)
	{
		// 윈도우 메시지가 있으면 윈도우 메시지 처리.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// 그렇지 않으면 애니메이션이나 게임 관련 작업을 수행.
		else
		{
			Timer.Tick();

			if (!bIsAppPaused)
			{
				CalculateFrameStats();
				UpdateScene(Timer.GetDeltaTime());
				DrawScene();
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

bool FD3DApp::Init()
{
	if (!InitMainWindow())
		return false;

	if (!InitDirect3D())
		return false;

	return true;
}

void FD3DApp::OnResize()
{
	assert(DeviceContext);
	assert(Device);
	assert(SwapChain);

	// 버퍼를 없애기 전에 버퍼들이 참조하고 있는 기존 뷰들을 해제.
	// 이후 Depth/Stencil buffer와 view 같이 해제.
	RenderTargetView.Reset();
	DepthStencilView.Reset();
	DepthStencilBuffer.Reset();

	// swap chian의 크기를 조정하고 RenderTargetView를 다시 생성.
	ThrowIfFailed(SwapChain->ResizeBuffers(1, CanvasWidth, CanvasHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ComPtr<ID3D11Texture2D> backBuffer;
	ThrowIfFailed(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf())));
	ThrowIfFailed(Device->CreateRenderTargetView(backBuffer.Get(), 0, &RenderTargetView));

	// Depth/Stencil buffer와 view 생성.
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = CanvasWidth;
	depthStencilDesc.Height = CanvasHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// swap chain의 MSAA values와 일치 해야함.
	if (bIs4xMsaaEnabled)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = Quality4xMsaa - 1;
	}
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	ThrowIfFailed(Device->CreateTexture2D(&depthStencilDesc, 0, DepthStencilBuffer.GetAddressOf()));
	ThrowIfFailed(Device->CreateDepthStencilView(DepthStencilBuffer.Get(), 0, DepthStencilView.GetAddressOf()));


	// RenderTargetView와 DepthStencilView를 그래픽 파이프라인에 연결.
	DeviceContext->OMSetRenderTargets(1, RenderTargetView.GetAddressOf(), DepthStencilView.Get());


	// Viewport transform 설정
	Viewport.TopLeftX = 0;
	Viewport.TopLeftY = 0;
	Viewport.Width = static_cast<float>(CanvasWidth);
	Viewport.Height = static_cast<float>(CanvasHeight);
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	DeviceContext->RSSetViewports(1, &Viewport);
}

LRESULT FD3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE 윈도우가 활성화 혹은 비활성화 될 때 보내지는 메시지.
		// 창이 비활성화되면 게임을 일시정지하고, 활성화되면 해제.
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			bIsAppPaused = true;
			Timer.Stop();
		}
		else
		{
			bIsAppPaused = false;
			Timer.Start();
		}
		return 0;
		// WM_SIZE 윈도우 크기를 조정할 때 보내지는 메시지.

	case WM_SIZE:
		// 변경된 창의 내부 영역 너비와 높이를 기록.
		CanvasWidth = LOWORD(lParam);
		CanvasHeight = HIWORD(lParam);
		if (Device)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				bIsAppPaused = true;
				bIsMinimized = true;
				bIsMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				bIsAppPaused = false;
				bIsMinimized = false;
				bIsMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (bIsMinimized)
				{
					bIsAppPaused = false;
					bIsMinimized = false;
					OnResize();
				}
				else if (bIsMaximized)
				{
					bIsAppPaused = false;
					bIsMaximized = false;
					OnResize();
				}
				else if (bResizing)
				{
					// 창 크기를 바꾸는 중에는 버퍼 크기를 조정하지 않는다.
					// 왜냐하면 계속 드래그하는 동안 창에는 WM_SIZE 메시지가 연속해서 계속 보내지는데,
					// 그때마다 버퍼 크기를 매번 다시 조정하는 것은 의미도 없고 느리기 때문이다.
					// 그래서 대신 사용자가 창 크기 조절을 끝내고 마우스를 놓아서 크기 조절 바를 해제한 뒤,
					// 즉 WM_EXITSIZEMOVE 메시지가 왔을 때 리셋한다.
				}
				else // SetWindowPos나 SwapChain->SetFullscreenState 같은 API 호출.
				{
					OnResize();
				}
			}
		}
		return 0;

		// WM_ENTERSIZEMOVE 크기 조절 바를 잡았을 때 보내지는 메시지 .
	case WM_ENTERSIZEMOVE:
		bIsAppPaused = true;
		bResizing = true;
		Timer.Stop();
		return 0;

		// WM_EXITSIZEMOVE 크기 조절 바를 놓았을 때 보내지는 메시지.
		// 여기서 변경된 윈도우 크기를 기준으로 재설정.
	case WM_EXITSIZEMOVE:
		bIsAppPaused = false;
		bResizing = false;
		Timer.Start();
		OnResize();
		return 0;

		// WM_DESTROY 창을 닫는 도중 보내지는 메시지.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// WM_MENUCHAR 메뉴를 띄운 상태에서, 메뉴 항목과 연결된 문자 키나 등록된 단축키가 아닌 다른 키를 누르면 보내지는 메시지.
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// 윈도우 크기가 지나치게 작아지지 않도록, 이 메시지를 가로채서 처리.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool FD3DApp::InitMainWindow()
{
	WNDCLASS wc = {};
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = AppInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = L"D3DWndClassName";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// 요청한 영역 크기를 기준으로, 윈도우 사각형의 크기를 계산.
	RECT R = { 0, 0, CanvasWidth, CanvasHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	AppWnd = CreateWindow(L"D3DWndClassName", MainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, AppInstance, 0);
	if (!AppWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(AppWnd, SW_SHOW);
	UpdateWindow(AppWnd);

	return true;
}

bool FD3DApp::InitDirect3D()
{
	// Device와 Device context 생성.

	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
		nullptr,                 // default adapter
		DriverType,
		nullptr,                 // no software Device
		createDeviceFlags,
		nullptr, 0,              // default feature level array
		D3D11_SDK_VERSION,
		&Device,
		&featureLevel,
		&DeviceContext);

	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	// 백 버퍼 형식에서 4x MSAA를 사용할 수 있는지 확인하는 코드.
	// Direct3D 11 지원 GPU라면 4x MSAA 기능 자체는 기본적으로 지원하므로,
	// 실제로는 몇 단계의 품질 옵션이 제공되는지만 확인하면 된다.
	ThrowIfFailed(Device->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM, 4, &Quality4xMsaa));
	assert(Quality4xMsaa > 0);

	// DXGI_SWAP_CHAIN_DESC을 사용자 정의로 설정.
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = CanvasWidth;
	sd.BufferDesc.Height = CanvasHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	if (bIs4xMsaaEnabled)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = Quality4xMsaa - 1;
	}
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = AppWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	// 스왑 체인을 올바르게 생성하려면, 디바이스를 만들 때 사용했던 IDXGIFactory 를 사용해야 한다.
	// 만약 CreateDXGIFactory를 호출해서 다른 IDXGIFactory 인스턴스 를 사용하려고 하면, 다음과 같은 오류가 발생한다 :
	// "IDXGIFactory::CreateSwapChain: This function is being called with a Device from a different IDXGIFactory."

	ComPtr<IDXGIDevice> dxgiDevice;
	ThrowIfFailed(Device->QueryInterface(__uuidof(IDXGIDevice), (void**)dxgiDevice.GetAddressOf()));

	ComPtr<IDXGIAdapter> dxgiAdapter;
	ThrowIfFailed(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)dxgiAdapter.GetAddressOf()));

	ComPtr<IDXGIFactory> dxgiFactory;
	ThrowIfFailed(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)dxgiFactory.GetAddressOf()));

	ThrowIfFailed(dxgiFactory->CreateSwapChain(Device.Get(), &sd, SwapChain.GetAddressOf()));

	// D3D 생성 과정에서 남아 있는 나머지 단계들은,
	// 창 크기 변경 시 다시 실행해야 하는 작업과 같다.
	// 따라서 코드 중복을 피하기 위해 여기서는 그냥 OnResize 메서드를 호출
	OnResize();

	return true;
}

void FD3DApp::CalculateFrameStats()
{
	// FPS와 프레임 하나를 렌더링하는 데 걸리는 평균 시간을 계산.
	// 그리고 그 값들을 윈도우 제목 표시줄에 덧붙인다.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ((Timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << MainWndCaption << L"    "
			<< L"FPS: " << fps << L"    "
			<< L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(AppWnd, outs.str().c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}


