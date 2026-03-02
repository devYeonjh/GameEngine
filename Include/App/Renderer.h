#pragma once

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

namespace App
{
    /**
     * 렌더링에 사용되는 단순 정점 구조체
     * POSITION(xyz) + COLOR(rgba) 레이아웃을 가짐
     */
    struct FVertexSimple
    {
        float x, y, z;		// Position
        float r, g, b, a;	// Color
    };

    /**
     * 렌더링 전용 3차원 벡터
     *
     * Engine::Vector3와 별도로 존재하며, 렌더링 레이어에서만 사용합니다.
     */
    struct FVector
    {
        float x, y, z;
        FVector(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

        FVector operator+(const FVector& rhs) const { return FVector(x + rhs.x, y + rhs.y, z + rhs.z); }
        FVector operator-(const FVector& rhs) const { return FVector(x - rhs.x, y - rhs.y, z - rhs.z); }
        FVector operator*(float s) const { return FVector(x * s, y * s, z * s); }
        FVector& operator+=(const FVector& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
        FVector& operator-=(const FVector& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
        FVector& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    };

    /**
      * Direct3D 11 렌더링 파이프라인 관리 클래스
      *
      * 디바이스 생성부터 프레임 출력까지 렌더링에 필요한
      * 모든 D3D11 리소스를 생성, 유지, 해제합니다.
      */
    class Renderer
    {
    public:
        /**
         * 기본 상수 버퍼 구조체
         *
         * 셰이더에 오브젝트의 위치(Offset)와 스케일(Pad)을 전달합니다.
         * 커스텀 상수 버퍼가 필요한 경우 UpdateConstantBuffer(void*, UINT)를 직접 사용하세요.
         */
        struct FConstants
        {
            FVector Offset;
            float   Pad;
        };

        // ------------------------------------------
        // D3D11 핵심 오브젝트
        // ------------------------------------------
        /** GPU와 통신하기 위한 Direct3D 11 디바이스 */
        ID3D11Device* Device = nullptr;
        
        /** GPU 명령 실행을 담당하는 컨텍스트 */
        ID3D11DeviceContext* DeviceContext = nullptr;
       
        /** 프레임 버퍼를 교체하는데 사용되는 스왑 체인 */
        IDXGISwapChain* SwapChain = nullptr;

        // ------------------------------------------
        // 렌더 타겟 및 뎁스 스탠실
        // ------------------------------------------
        /** 화면 출력용 텍스처 */
        ID3D11Texture2D* FrameBuffer = nullptr;
        
        /** 텍스처를 렌더 타겟으로 사용하는 뷰 */
        ID3D11RenderTargetView* FrameBufferRTV = nullptr;
        
        /**
         * 뎁스 스탠실 뷰
         * Create() 호출 시 useDepth=true인 경우에만 생성.
         */
        ID3D11DepthStencilView* DepthStencilView = nullptr;
        
        /**
         * 뎁스 스탠실 버퍼 텍스처
         * Create() 호출 시 useDepth=true인 경우에만 생성.
         */
        ID3D11Texture2D* DepthStencilBuffer = nullptr;
        
        /** 래스터라이저 상태(컬링, 채우기 모드 등 정의) */
        ID3D11RasterizerState* RasterizerState = nullptr;
        
        /** 셰이더에 데이터를 전달하기 위한 상수 버퍼 */
        ID3D11Buffer* ConstantBuffer = nullptr;

        /** 화면을 초기화(clear)할 때 사용할 색상 (RGBA) */
        FLOAT          ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
        
        /** 렌더링 영역을 정의하는 뷰포트 정보 */
        D3D11_VIEWPORT ViewportInfo = {};

        // ------------------------------------------
        // 셰이더 관련
        // ------------------------------------------
        ID3D11VertexShader* SimpleVertexShader = nullptr;
        ID3D11PixelShader* SimplePixelShader = nullptr;
        ID3D11InputLayout* SimpleInputLayout = nullptr;
        UINT                Stride = 0;

    public:
        /**
         * 렌더러를 초기화합니다.
         *
         * 디바이스, 스왑체인, 프레임버퍼, 래스터라이저를 순서대로 생성합니다.
         */
        void Create(HWND hWindow, bool useDepth = false);

        /**
         * 렌더러가 소유한 모든 D3D11 리소스를 해제합니다.
         *
         * 셰이더, 상수 버퍼, 프레임 버퍼, 디바이스 순으로 해제하며
         * 애플리케이션 종료 전에 반드시 호출해야 합니다.
         */
        void Release();

        /**
         * 윈도우 크기 변경 시 렌더 타겟과 뷰포트를 재생성합니다.
         *
         * WM_SIZE 메시지를 수신했을 때 호출하세요.
         * 깊이 버퍼가 있다면 함께 재생성합니다.
         */
        void OnResize(HWND hWindow);

        /**
         * HLSL 파일로부터 버텍스·픽셀 셰이더를 컴파일하고 생성합니다.
         *
         * 셰이더 파일 내에 mainVS(vs_5_0)와 mainPS(ps_5_0) 진입점이
         * 반드시 정의되어 있어야 합니다.
         * 입력 레이아웃은 FVertexSimple(POSITION, COLOR) 기준으로 자동 설정됩니다.
         */
        void CreateShader(const wchar_t* shaderPath);

        /**
         * 셰이더 및 입력 레이아웃을 해제합니다.
         */
        void ReleaseShader();

        /**
         * 상수 버퍼를 생성합니다.
         *
         * 내부적으로 16바이트 정렬을 보장합니다.
         * 커스텀 구조체를 사용하는 경우 sizeof(MyStruct)를 전달하세요.
         */
        void CreateConstantBuffer(UINT byteSize = sizeof(FConstants));
        
        /**
         * 상수 버퍼를 해제합니다.
         */
        void ReleaseConstantBuffer();

        /**
         * 상수 버퍼를 임의 데이터로 갱신합니다.
         *
         * 커스텀 상수 버퍼 구조체를 사용할 때 이 함수를 직접 호출하세요.
         */
        void UpdateConstantBuffer(const void* data, UINT byteSize);

        /**
         * 기본 상수 버퍼(FConstants)를 갱신하는 편의 오버로드입니다.
         */
        void UpdateConstantBuffer(const FConstants& constants)
        {
            UpdateConstantBuffer(&constants, sizeof(FConstants));
        }

        /**
        * GPU에 정점 데이터를 업로드하고 버텍스 버퍼를 생성합니다.
        *
        * 생성된 버퍼는 IMMUTABLE(변경 불가)로 설정됩니다.
        * 반환된 버퍼는 사용 후 반드시 ReleaseVertexBuffer()로 해제해야 합니다.
        */
        ID3D11Buffer* CreateVertexBuffer(const void* vertices, UINT byteWidth);
        
        /**
         * CreateVertexBuffer()로 생성한 버퍼를 해제합니다.
         */
        void          ReleaseVertexBuffer(ID3D11Buffer* buffer);

        /**
         * 새 프레임 렌더링을 시작합니다.
         *
         * 렌더 타겟을 ClearColor로 초기화하고 뷰포트, 래스터라이저,
         * 렌더 타겟 바인딩을 설정합니다.
         * 매 프레임 가장 먼저 호출해야 합니다.
         */
        void BeginFrame();

        /**
         * 셰이더와 상수 버퍼를 파이프라인에 바인딩합니다.
         *
         * BeginFrame() 이후, RenderPrimitive() 이전에 호출합니다.
         * 셰이더가 CreateShader()로 생성되어 있어야 합니다.
         */
        void PrepareShader();

        /**
         * 버텍스 버퍼를 바인딩하고 드로우콜을 실행합니다.
         *
         * PrepareShader() 이후에 호출해야 합니다.
         */
        void RenderPrimitive(ID3D11Buffer* vertexBuffer, UINT numVertices);

        /**
         * 현재 프레임을 화면에 출력하고 버퍼를 교환합니다.
         *
         * ImGui 렌더링이 완료된 후 마지막으로 호출해야 합니다.
         */
        void EndFrame();

    private:
        /** Direct3D 장치 및 스왑체인 생성 */
        void CreateDeviceAndSwapChain(HWND hWindow);
        /** 프레임 버퍼 생성 */
        void CreateFrameBuffer();
        /** 깊이 스탠실 버퍼 생성 */
        void CreateDepthStencilBuffer(UINT width, UINT height);
        /** 래스터라이저 상태 생성 */
        void CreateRasterizerState();

        void ReleaseDeviceAndSwapChain();
        void ReleaseFrameBuffer();
    };
}


