add_library(EngineCore STATIC
  Particle.cpp
)

target_link_libraries(EngineCore PUBLIC ge_warnings)

target_include_directories(EngineCore PUBLIC
  "${CMAKE_SOURCE_DIR}/Include"
)

ge_set_outputs(EngineCore)

if(MSVC)
  set_target_properties(EngineCore PROPERTIES
    VS_GLOBAL_OutDir "${CMAKE_SOURCE_DIR}/Lib/$(Configuration)\\"
  )
endif()