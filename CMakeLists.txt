cmake_minimum_required(VERSION 3.24)

project(GameEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------
# Output layout: Bin/<Target>/<Config>, Lib/<Config>
# ------------------------------------------------------------
set(GE_BIN_DIR "${CMAKE_SOURCE_DIR}/Bin")
set(GE_LIB_DIR "${CMAKE_SOURCE_DIR}/Lib")

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
      RUNTIME_OUTPUT_DIRECTORY_${cfg_u} "${GE_BIN_DIR}/${target}"
    )
  endforeach()
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
# Executable helper functions
# ------------------------------------------------------------
function(ge_add_console_exe target)
  add_executable(${target} ${ARGN})
  target_link_libraries(${target} PRIVATE ge_warnings)
  ge_set_outputs(${target})
endfunction()

function(ge_add_win32_dx11_exe target)
  add_executable(${target} WIN32
    "${CMAKE_SOURCE_DIR}/Src/Exercises/Main.cpp"
    ${ARGN}
  )
  target_link_libraries(${target} PRIVATE 
    ge_warnings 
    Framework
    user32 
    d3dcompiler
  )
  target_compile_definitions(${target} PRIVATE UNICODE _UNICODE)
  ge_set_outputs(${target})
endfunction()

# ------------------------------------------------------------
# Subdirectories
# ------------------------------------------------------------
add_subdirectory(Src/Engine)
add_subdirectory(Src/Exercises)
add_subdirectory(Src/Framework)