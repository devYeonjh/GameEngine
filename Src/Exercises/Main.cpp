#include "D3DApp.h"

using namespace Framework;

namespace
{
	// 전역 윈도우 프로시저에서 멤버 함수 윈도우 프로시저로 Windows 메시지를 전달하기 위해 사용된다.
	// 멤버 함수는 WNDCLASS::lpfnWndProc에 직접 할당할 수 없기 때문.
	std::unique_ptr<FD3DApp> GD3DApp;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// hwnd를 그대로 전달해 둔다. 왜냐하면 CreateWindow가 반환되기 전에,
	// 즉 AppWnd가 아직 유효해지기 전에 WM_CREATE 같은 메시지를 받을 수 있기 때문이다.
	return GD3DApp->MsgProc(hwnd, msg, wParam, lParam);
}

// 애플리케이션 객체를 반환할 함수의 전방 선언.
// 이 함수는 Demo.cpp 혹은 Exercise.cpp 파일에서 구현되어야 함.
extern std::unique_ptr<Framework::FD3DApp> GetApplication(HINSTANCE hInstance);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
    GD3DApp = GetApplication(hInstance);
    GD3DApp->Init();
    return GD3DApp->Run();
}