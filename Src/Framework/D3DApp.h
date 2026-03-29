#pragma once

#include "pch.h"
#include "Timer.h" 

namespace Framework {

	class FD3DApp
	{
	public:
		FD3DApp(HINSTANCE hInstance);
		virtual ~FD3DApp() = default;

		FD3DApp(const FD3DApp&) = delete;
		FD3DApp& operator=(const FD3DApp&) = delete;
		
		FD3DApp(FD3DApp&&) = delete;
		FD3DApp& operator=(FD3DApp&&) = delete;

		HINSTANCE GetAppInstance() const;
		HWND      GetAppWnd() const;
		float     GetAspectRatio() const;

		int Run();

		// Framework Method
		// 파생된 클래스에서 구현 요구사항에 맞게 재정의한다.
		virtual bool Init();
		virtual void OnResize();
		virtual void UpdateScene(float dt) = 0;
		virtual void DrawScene() = 0;
		virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		// 마우스 입력 처리를 위한 편의용 함수.
		virtual void OnMouseDown(WPARAM btnState, int x, int y) {}
		virtual void OnMouseUp(WPARAM btnState, int x, int y) {}
		virtual void OnMouseMove(WPARAM btnState, int x, int y) {}

	protected:
		bool InitMainWindow();
		bool InitDirect3D();

		void CalculateFrameStats();

	protected:
		HINSTANCE AppInstance;
		HWND      AppWnd;
		bool      bIsAppPaused;
		bool      bIsMinimized;
		bool      bIsMaximized;
		bool      bResizing;
		UINT      Quality4xMsaa;

		FTimer Timer;

		Microsoft::WRL::ComPtr<ID3D11Device> Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> DepthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView;
		D3D11_VIEWPORT Viewport;

		// 파생 클래스에서 시작값을 변경하려면 생성자에서 이 값들을 설정해야 한다.
		std::wstring MainWndCaption;
		D3D_DRIVER_TYPE DriverType;
		int CanvasWidth;
		int CanvasHeight;
		bool bIs4xMsaaEnabled;
	};

}
