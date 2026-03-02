cmake_minimum_required(VERSION 3.24)

project(GameEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------
# Output layout: Bin/<Target>/<Config>, Lib/<Config>, Obj/<Target>/<Config>
# ------------------------------------------------------------
set(GE_BIN_DIR "${CMAKE_SOURCE_DIR}/Bin")
set(GE_LIB_DIR "${CMAKE_SOURCE_DIR}/Lib")
set(GE_OBJ_DIR "${CMAKE_SOURCE_DIR}/Obj")

function(ge_set_outputs target)
  set_target_properties(${target} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${GE_LIB_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${GE_LIB_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${GE_BIN_DIR}/${target}"
  )

  foreach(cfg Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${cfg}" cfg_u)
    set_target_properties(${target} PROPERTIES
      ARCHIVE_OUTPUT_DIRECTORY_${cfg_u} "${GE_LIB_DIR}"
      LIBRARY_OUTPUT_DIRECTORY_${cfg_u} "${GE_LIB_DIR}"
      RUNTIME_OUTPUT_DIRECTORY_${cfg_u} "${GE_BIN_DIR}/${target}/${cfg}"
    )
  endforeach()

  # Visual Studio(MSBuild) 중간 산출물/출력 디렉토리 강제 통일
  if(MSVC)
    set_target_properties(${target} PROPERTIES
      # Obj/<Target>/<Config>/
      VS_GLOBAL_IntDir "${GE_OBJ_DIR}/${target}/$(Configuration)\\"
      # Bin/<Target>/<Config>/  (exe 타겟에선 이미 RUNTIME_OUTPUT_DIRECTORY로도 맞지만 통일용)
      VS_GLOBAL_OutDir "${GE_BIN_DIR}/${target}/$(Configuration)\\"
    )
  endif()
endfunction()

# ------------------------------------------------------------
# Common compile options
# ------------------------------------------------------------
add_library(ge_warnings INTERFACE)
if(MSVC)
  target_compile_options(ge_warnings INTERFACE /W4 /permissive- /EHsc)
else()
  target_compile_options(ge_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

# ------------------------------------------------------------
# Subdirectories
# ------------------------------------------------------------
add_subdirectory(Src/EngineCore)
add_subdirectory(Src/AppFramework)
add_subdirectory(Src/Exercises)
add_subdirectory(Src/Demos)