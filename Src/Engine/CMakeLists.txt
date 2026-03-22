add_library(Engine STATIC
  Physics/Particle.cpp
)

target_link_libraries(Engine PUBLIC ge_warnings)

target_include_directories(Engine PUBLIC
  "${CMAKE_SOURCE_DIR}/Src/Engine"
)

ge_set_outputs(Engine)

if(MSVC)
  set_target_properties(Engine PROPERTIES
    VS_GLOBAL_OutDir "${CMAKE_SOURCE_DIR}/Lib/$(Configuration)\\"
  )
endif()