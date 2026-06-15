#pragma once

#include "pch.h"
#include "Timer.h"
#include "D3Dutil.h"

namespace Framework {

	class FD3DApp
	{
	public:
		// 생성자의 암시적 형변환을 막음
		explicit FD3DApp(HINSTANCE hInstance);
		virtual ~FD3DApp() = default;

		// 윈도우 핸들, Direct3D 디바이스, 스왑 체인처럼 프로세스/OS 리소스와
		// 직접 연결된 객체를 소유하므로 애플리케이션 인스턴스의 복사와 이동을 금지한다.
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

		/**
		 * Window 및 DirectX 초기화
		 * InitMainWindow()로 Win32 창을 생성하고,
		 * InitDirect3D()로 Device, DeviceContext, SwapChain,
		 * RenderTargetView, DepthStencilView를 초기화한다.
		 */
		virtual bool Init();		
		/**
		 * 창 크기 변경 시 호출되는 함수.
		 * SwapChain의 버퍼 크기를 재조정하고,
		 * RenderTargetView, DepthStencilBuffer, DepthStencilView를 재생성한다.
		 * 또한 Viewport를 현재 CanvasWidth/CanvasHeight에 맞게 재설정한다.
		 * 초기화 시 InitDirect3D() 내부에서도 호출된다.
		 */
		virtual void OnResize();
		/**
		 * 매 프레임마다 호출되어 Scene의 상태를 갱신한다.
		 * @param dt 이전 프레임과의 시간 간격 (초 단위, delta time)
		 * 물리, 애니메이션, 입력 처리 등 게임 로직을 이곳에서 구현한다.
		 */
		virtual void UpdateScene(float dt) = 0;
		/**
		 * 매 프레임마다 호출되어 Scene을 렌더링한다.
		 */
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
