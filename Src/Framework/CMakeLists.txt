add_library(Framework STATIC
	"D3DApp.cpp"
	"D3DApp.h"
    "Timer.cpp"
	"Timer.h"
	"pch.h"  "DebugLog.h")

target_precompile_headers(Framework PRIVATE pch.h)

target_compile_definitions(Framework PRIVATE UNICODE _UNICODE)

target_link_libraries(Framework PUBLIC
  ge_warnings
  Engine
  d3d11
  dxgi
  d3dcompiler
)

target_include_directories(Framework PUBLIC
  "${CMAKE_SOURCE_DIR}/Src/Framework"
)

ge_set_outputs(Framework)