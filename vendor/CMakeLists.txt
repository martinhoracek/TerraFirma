set(SDL_TEST_LIBRARY OFF)
add_subdirectory(SDL)

add_library(vendor INTERFACE)
target_link_libraries(vendor INTERFACE SDL3::SDL3)

target_include_directories(vendor INTERFACE
  imgui
  imgui/backends
  ImGuiFileDialog/
  glm
)

target_sources(vendor INTERFACE
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/backends/imgui_impl_sdlgpu3.cpp
  imgui/backends/imgui_impl_sdl3.cpp
  imgui/misc/cpp/imgui_stdlib.cpp
  ImGuiFileDialog/ImGuiFileDialog.cpp
)
