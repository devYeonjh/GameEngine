# ------------------------------------------------------------
# Dx11Deps: DX11 링크 라이브러리 묶음 (Shared Project 역할의 일부)
# ------------------------------------------------------------
add_library(Dx11Deps INTERFACE)

if(WIN32)
  target_link_libraries(Dx11Deps INTERFACE
    d3d11
    user32
    d3dcompiler
  )
endif()

# ------------------------------------------------------------
# AppFrameworkObj: 공유 소스(OBJECT library)
#  - Win32/DX11/ImGui 공통 코드를 한 번 컴파일해서 여러 exe가 재사용
# ------------------------------------------------------------
add_library(AppFrameworkObj OBJECT)

# ImGui (현재 ThirdParty/ImGui 구조에 맞춰 명시)
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/ImGui")

set(IMGUI_CORE_CPP
  "${IMGUI_DIR}/imgui.cpp"
  "${IMGUI_DIR}/imgui_draw.cpp"
  "${IMGUI_DIR}/imgui_tables.cpp"
  "${IMGUI_DIR}/imgui_widgets.cpp"
  # "${IMGUI_DIR}/imgui_demo.cpp"  # 필요하면 켜기
)

set(IMGUI_BACKEND_CPP
  "${IMGUI_DIR}/imgui_impl_win32.cpp"
  "${IMGUI_DIR}/imgui_impl_dx11.cpp"
)

target_sources(AppFrameworkObj PRIVATE
  ${IMGUI_CORE_CPP}
  ${IMGUI_BACKEND_CPP}
 "Application.cpp" "Renderer.cpp" "Timing.cpp")

# 경고는 AppFramework에도 적용(원하면 PRIVATE로 둬도 됨)
target_link_libraries(AppFrameworkObj PRIVATE
  ge_warnings
  Dx11Deps
)

target_include_directories(AppFrameworkObj PUBLIC
  "${CMAKE_SOURCE_DIR}/Include"
  "${IMGUI_DIR}"
)

if(WIN32)
  target_compile_definitions(AppFrameworkObj PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
  )
endif()