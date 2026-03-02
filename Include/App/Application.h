#pragma once

#include "Renderer.h"

namespace App
{
    /**
     * Win32 메시지(키보드, 마우스, 리사이즈)와 렌더 루프(update/display/renderUI)를
     * 가상 함수로 제공합니다.
     *
     * 구체적인 데모는 이 클래스를 상속받아 필요한 함수만 오버라이드합니다.
     * Renderer는 initGraphics()에서 주입받으며 포인터로 보유합니다.
     */
    class Application
    {
    protected:
        /** 현재 렌더링 영역의 너비 (픽셀) */
        int Width = 0;

        /** 현재 렌더링 영역의 높이 (픽셀) */
        int Height = 0;

        /**
         * 렌더러 포인터
         *
         * initGraphics() 호출 시 외부에서 주입됩니다.
         * 소유권은 Application에 없으므로 직접 해제하지 않습니다.
         */
        Renderer* renderer = nullptr;

    public:
        virtual ~Application() = default;

        /**
         * 윈도우 타이틀바에 표시할 데모 이름을 반환합니다.
         *
         * 기본값은 "Demo"이며 각 데모에서 오버라이드할 수 있습니다.
         */
        virtual const char* getTitle() { return "Demo"; }

        /**
         * 렌더러를 주입받고 그래픽 리소스를 초기화합니다.
         *
         * main.cpp의 초기화 단계에서 1회 호출됩니다.
         * 셰이더 생성, 버텍스 버퍼 생성 등을 여기에서 수행하세요.
         */
        virtual void initGraphics(Renderer& r) { renderer = &r; }

        /**
         * 종료 시 그래픽 리소스를 정리합니다.
         *
         * Release() 계열 함수 호출은 이곳에서 수행합니다.
         */
        virtual void deinit() {}

        /**
         * 매 프레임 시뮬레이션 상태를 갱신합니다.
         *
         * 물리 계산, 오브젝트 이동 등 렌더링 이전의 로직을 구현합니다.
         * TimingData::get().GetDeltaTime()으로 프레임 시간을 참조하세요.
         */
        virtual void update() {}

        /**
         * 매 프레임 씬을 렌더링합니다.
         *
         * renderer->BeginFrame() / EndFrame() 사이에서 호출되므로
         * 이 함수 내에서는 D3D11 드로우콜만 작성하면 됩니다.
         */
        virtual void display() {}

        /**
         * 매 프레임 ImGui UI를 렌더링합니다.
         *
         * ImGui::NewFrame() / ImGui::Render() 사이에서 호출됩니다.
         * ImGui::Begin() ~ ImGui::End() 블록을 이 함수 안에서 작성하세요.
         */
        virtual void renderUI() {}

        // ------------------------------------------
        // 입력 이벤트
        // ------------------------------------------

        /**
         * 키보드 입력 이벤트를 처리합니다.
         *
         * WndProc의 WM_KEYDOWN에서 호출됩니다.
         */
        virtual void key(unsigned char key) {}

        /**
         * 윈도우 크기 변경 이벤트를 처리합니다.
         *
         * WndProc의 WM_SIZE에서 호출됩니다.
         * 기본 구현은 Width, Height를 갱신하고 Renderer::OnResize()를 호출합니다.
         * 오버라이드 시 부모 구현을 함께 호출하는 것을 권장합니다.
         */
        virtual void resize(int width, int height);

        /**
         * 마우스 버튼 이벤트를 처리합니다.
         *
         * WndProc의 WM_LBUTTONDOWN / WM_RBUTTONDOWN에서 호출됩니다.
         * 기본 구현은 아무것도 하지 않습니다.
         */
        virtual void mouse(int button, int state, int x, int y) {}

        /**
         * 마우스 드래그 이벤트를 처리합니다.
         *
         * WndProc의 WM_MOUSEMOVE에서 버튼이 눌린 상태일 때 호출됩니다.
         */
        virtual void mouseDrag(int x, int y) {}
    };

    /**
     * Application을 상속받아 카메라 회전, 일시정지, 접촉 디버그 렌더링 등
     * 강체 데모에서 공통으로 필요한 기능을 제공합니다.
     *
     * 순수 가상 함수인 generateContacts(), updateObjects(), reset()을
     * 반드시 구체 데모 클래스에서 구현해야 합니다.
     */
    class RigidBodyApplication : public Application
    {
    protected:
        // ------------------------------------------
        // 카메라
        // ------------------------------------------

        /** 카메라 수평 회전각 (도 단위, 마우스 드래그로 조작) */
        float theta = 0.0f;

        /** 카메라 수직 회전각 (도 단위, -20 ~ 80으로 클램핑) */
        float phi = 15.0f;

        /** 마우스 드래그 직전 프레임의 X 좌표 */
        int last_x = 0;

        /** 마우스 드래그 직전 프레임의 Y 좌표 */
        int last_y = 0;

        // ------------------------------------------
        // 시뮬레이션 제어
        // ------------------------------------------

        /** true이면 시뮬레이션이 일시정지 상태 */
        bool pauseSimulation = true;

        /**
         * true이면 다음 프레임 처리 후 자동으로 일시정지
         * 스페이스 키로 1프레임씩 진행할 때 사용합니다.
         */
        bool autoPauseSimulation = false;

        /** true이면 접촉점과 법선 벡터를 디버그 렌더링 */
        bool renderDebugInfo = false;

    protected:
        /**
         * 접촉 생성 로직을 처리합니다.
         *
         * update() 내부에서 매 프레임 호출됩니다.
         * cData에 접촉 정보를 채워야 합니다.
         */
        virtual void generateContacts() = 0;

        /**
         * 강체 오브젝트를 duration 만큼 앞으로 진행합니다.
         *
         * update() 내부에서 generateContacts() 이전에 호출됩니다.
         */
        virtual void updateObjects(float duration) = 0;

        /**
         * 시뮬레이션을 초기 상태로 리셋합니다.
         *
         * R 키 입력 시 호출됩니다.
         */
        virtual void reset() = 0;

    public:
        RigidBodyApplication() = default;

        /**
         * 강체 시뮬레이션 상태를 1프레임 갱신합니다.
         *
         * 일시정지 상태가 아닐 때: updateObjects → generateContacts → resolveContacts 순으로 처리합니다.
         * 일시정지 상태일 때: 아무 작업도 수행하지 않습니다.
         */
        virtual void update() override;

        /**
         * 카메라 행렬을 상수 버퍼에 업데이트합니다.
         *
         * 구체 데모에서 오버라이드 시 부모 구현을 먼저 호출하거나
         * theta/phi를 직접 사용해 뷰 행렬을 계산하세요.
         */
        virtual void display() override;

        /**
         * 마우스 버튼 이벤트를 처리하여 드래그 기준점을 저장합니다.
         */
        virtual void mouse(int button, int state, int x, int y) override;

        /**
         * 마우스 드래그로 카메라를 회전시킵니다.
         *
         * theta(수평)와 phi(수직)를 갱신하며
         * phi는 -20 ~ 80도로 클램핑됩니다.
         */
        virtual void mouseDrag(int x, int y) override;

        /**
         * 키 입력을 처리합니다.
         *
         * R: 리셋, P: 일시정지 토글, Space: 1프레임 진행, C: 디버그 토글
         */
        virtual void key(unsigned char key) override;
    };

}
