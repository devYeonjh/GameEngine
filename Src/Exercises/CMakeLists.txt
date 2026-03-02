# 공통: Exercise exe를 만드는 함수
function(add_exercise target_name dir)
  add_executable(${target_name})
  target_sources(${target_name} PRIVATE
    $<TARGET_OBJECTS:AppFrameworkObj>
    "${CMAKE_SOURCE_DIR}/Src/main.cpp"
    "${dir}/${target_name}.cpp"
  )
  target_link_libraries(${target_name} PRIVATE
    EngineCore
    Dx11Deps
    ge_warnings
  )
  target_include_directories(${target_name} PRIVATE
    "${CMAKE_SOURCE_DIR}/Include"
    "${CMAKE_SOURCE_DIR}/ThirdParty/ImGui"
    "${CMAKE_SOURCE_DIR}/Src/AppFramework"
  )
  # 출력 규칙 적용
  ge_set_outputs(${target_name})
endfunction()

# -----------------------
# Exercise 등록
# -----------------------
# add_exercise(Exercise_3_5 "${CMAKE_CURRENT_SOURCE_DIR}/Exercise3_5")