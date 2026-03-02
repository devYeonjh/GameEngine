#include "App/Application.h"
#include "App/Timing.h"

using namespace App;

void Application::resize(int width, int height)
{
    // 0 나누기 방지 (Cyclone 원본과 동일)
    if (height <= 0) height = 1;

    Width = width;
    Height = height;
}

void RigidBodyApplication::update()
{
    float duration = Timing::Get().GetDeltaTime();
    if (duration <= 0.0f) return;

    // 일시정지 상태이면 시뮬레이션을 진행하지 않음
    if (pauseSimulation)
    {
        return;
    }

    // 1프레임 진행 모드: 이번 프레임 처리 후 자동으로 일시정지
    if (autoPauseSimulation)
    {
        pauseSimulation = true;
        autoPauseSimulation = false;
    }

    // 물리 시뮬레이션 갱신 순서
    // 1) 오브젝트 위치·속도 적분
    updateObjects(duration);

    // 2) 접촉 생성
    generateContacts();

    // 3) 접촉 해소 (구체 데모에서 resolver를 초기화한 뒤 호출)
    //    resolver.resolveContacts(cData.contactArray, cData.contactCount, duration);
}

void RigidBodyApplication::display()
{
    // 카메라 파라미터(theta, phi)를 상수 버퍼에 반영하는 작업은
    // 구체 데모에서 행렬 라이브러리와 함께 오버라이드하여 구현합니다.
    //
    // 예시 (DirectXMath 사용 시):
    // XMMATRIX view = XMMatrixLookAtLH(eye, target, up);
    // XMMATRIX view = XMMatrixRotationY(XMConvertToRadians(theta)) * ...;
    // renderer->UpdateConstantBuffer(viewProj);
}

void RigidBodyApplication::mouse(int button, int state, int x, int y)
{
    // 드래그 기준점 저장 (Cyclone 원본과 동일)
    last_x = x;
    last_y = y;
}

void RigidBodyApplication::mouseDrag(int x, int y)
{
    // 수평·수직 회전각 갱신 (Cyclone 원본과 동일)
    theta += (x - last_x) * 0.25f;
    phi += (y - last_y) * 0.25f;

    // 수직 회전각 클램핑
    if (phi < -20.0f) phi = -20.0f;
    else if (phi > 80.0f) phi = 80.0f;

    last_x = x;
    last_y = y;
}

void RigidBodyApplication::key(unsigned char key)
{
    switch (key)
    {
    case 'R': case 'r':
        // 시뮬레이션 리셋
        reset();
        break;

    case 'P': case 'p':
        // 일시정지 토글
        pauseSimulation = !pauseSimulation;
        break;

    case ' ':
        // 1프레임 진행 (일시정지 상태에서 스텝 실행)
        autoPauseSimulation = true;
        pauseSimulation = false;
        break;

    case 'C': case 'c':
        // 접촉점 디버그 렌더링 토글
        renderDebugInfo = !renderDebugInfo;
        break;

    default:
        Application::key(key);
        break;
    }
}
