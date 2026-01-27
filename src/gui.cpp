/** @copyright 2025 Sean Kasun */

#include "gui.h"
#include "ttfs.h"
#include "filedialogfont.h"
#include "filedialogfont.cpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

SDL_GPUDevice *GUI::init() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
    SDLFAIL();
  }

  float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

  window = SDL_CreateWindow("Terrafirma", static_cast<int>(1280 * scale), static_cast<int>(720 * scale),
    SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY);
  if (!window) {
    SDLFAIL();
  }

  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(window);

  gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_METALLIB | SDL_GPU_SHADERFORMAT_DXIL, true, nullptr);
  if (!gpu) {
    SDLFAIL();
  }
  if (!SDL_ClaimWindowForGPUDevice(gpu, window)) {
    SDLFAIL();
  }

  // desired modes and their fallbacks
  SDL_GPUSwapchainComposition composition =SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR;
  SDL_GPUPresentMode presentMode = SDL_GPU_PRESENTMODE_MAILBOX;
  if (!SDL_WindowSupportsGPUSwapchainComposition(gpu, window, composition)) {
    // fallback always supported
    composition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
  }
  if (!SDL_WindowSupportsGPUPresentMode(gpu, window, presentMode)) {
    // fallback always supporte
    presentMode = SDL_GPU_PRESENTMODE_VSYNC;
  }

  SDL_SetGPUSwapchainParameters(gpu, window, composition, presentMode);

  drawImage = nullptr;
  depthImage = nullptr;

  initImgui(scale);
  return gpu;
}

void GUI::initImgui(float scale) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  auto &io = ImGui::GetIO();
  ImFontConfig fontCfg;
  fontCfg.FontDataOwnedByAtlas = false;  // this wasn't allocated, don't free it
  io.Fonts->AddFontFromMemoryTTF((void*)mplus_1m_regular_ttf, mplus_1m_regular_ttf_length, 0.0f, &fontCfg);
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  static const ImWchar iconRanges[] = { ICON_MIN_IGFD, ICON_MAX_IGFD, 0 };
  ImFontConfig iconConfig;
  iconConfig.MergeMode = true;
  iconConfig.PixelSnapH = true;
  io.Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_IGFD, 15.0f, &iconConfig, iconRanges);
  

  ImGui::StyleColorsDark();
  auto &style = ImGui::GetStyle();
  style.ScaleAllSizes(scale);
  style.FontScaleDpi = scale;

  ImGui_ImplSDL3_InitForSDLGPU(window);
  ImGui_ImplSDLGPU3_InitInfo info = {};
  info.Device = gpu;
  info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu, window);
  info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
  ImGui_ImplSDLGPU3_Init(&info);
}

void GUI::resizeSwapchain(Map *map) {

  int dpiWidth, dpiHeight;
  if (!SDL_GetWindowSize(window, &dpiWidth, &dpiHeight)) {
    SDLFAIL();
  }
  // we use scaled size here because mouse isn't hidpi
  map->setSize(dpiWidth, dpiHeight);

  if (!SDL_GetWindowSizeInPixels(window, &winWidth, &winHeight)) {
    SDLFAIL();
  }

  if (drawImage) {
    SDL_ReleaseGPUTexture(gpu, drawImage);
  }
  SDL_GPUTextureCreateInfo textureInfo = {
    .type = SDL_GPU_TEXTURETYPE_2D,
    .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB,
    .usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,
    .width = static_cast<uint32_t>(winWidth),
    .height = static_cast<uint32_t>(winHeight),
    .layer_count_or_depth = 1,
    .num_levels = 1,
  };
  drawImage = SDL_CreateGPUTexture(gpu, &textureInfo);
  if (!drawImage) {
    SDLFAIL();
  }

  if (depthImage) {
    SDL_ReleaseGPUTexture(gpu, depthImage);
  }
  SDL_GPUTextureCreateInfo depthInfo = {
    .type = SDL_GPU_TEXTURETYPE_2D,
    .format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
    .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
    .width = static_cast<uint32_t>(winWidth),
    .height = static_cast<uint32_t>(winHeight),
    .layer_count_or_depth = 1,
    .num_levels = 1,
    .sample_count = SDL_GPU_SAMPLECOUNT_1,
  };
  depthImage = SDL_CreateGPUTexture(gpu, &depthInfo);
  if (!depthImage) {
    SDLFAIL();
  }
}

bool GUI::processEvents(SDL_Event *event) {
  // io.WantCaptureMouse means don't send mouse to main
  // io.WantCaptureKeyboard means don't send keyboard to main
  ImGui_ImplSDL3_ProcessEvent(event);
  switch (event->type) {
    case SDL_EVENT_QUIT:
      return true;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
      if (event->window.windowID == SDL_GetWindowID(window)) {
        return true;
      }
      break;
  }
  return false;
}

bool GUI::fence() {
  if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
    SDL_Delay(10);
    return true;
  }
  if (renderFence) {
    if (!SDL_WaitForGPUFences(gpu, false, &renderFence, 1)) {
      SDLFAIL();
    }
    SDL_ReleaseGPUFence(gpu, renderFence);
    renderFence = nullptr;
  }

  ImGui_ImplSDLGPU3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  return false;
}

void GUI::render(Map *map) {
  ImDrawData *drawData = ImGui::GetDrawData();
  SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu);
  SDL_GPUTexture *swapchain;
  if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &swapchain, nullptr, nullptr)) {
    SDLFAIL();
  }
  if (swapchain) {
    SDL_GPUCopyPass *copy = SDL_BeginGPUCopyPass(cmd);
    map->copy(gpu, copy);
    SDL_EndGPUCopyPass(copy);

    SDL_GPUColorTargetInfo colorTarget[] = {
      {
        .texture = swapchain,
        .clear_color = SDL_FColor { 0.0, 0.0, 0.0, 1.0 },
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE,
      },
    };
    SDL_GPUDepthStencilTargetInfo depthTarget = {
      .texture = depthImage,
      .clear_depth = 0,
      .load_op = SDL_GPU_LOADOP_CLEAR,
      .store_op = SDL_GPU_STOREOP_STORE,
      .stencil_load_op = SDL_GPU_LOADOP_CLEAR,
      .stencil_store_op = SDL_GPU_STOREOP_STORE,
      .cycle = false,
      .clear_stencil = 0,
    };

    SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(cmd, colorTarget, SDL_arraysize(colorTarget), &depthTarget);
    SDL_GPUViewport viewport = {
      .x = 0,
      .y = 0,
      .w = static_cast<float>(winWidth),
      .h = static_cast<float>(winHeight),
      .min_depth = 0.f,
      .max_depth = 20.f,
    };
    SDL_SetGPUViewport(renderPass, &viewport);
    SDL_Rect scissor = {
      .x = 0,
      .y = 0,
      .w = winWidth,
      .h = winHeight,
    };
    SDL_SetGPUScissor(renderPass, &scissor);

    map->render(cmd, renderPass);

    SDL_EndGPURenderPass(renderPass);

    SDL_GPUColorTargetInfo imguiColorTarget[] = {
      {
        .texture = swapchain,
        .load_op = SDL_GPU_LOADOP_LOAD,  // overlay
        .store_op = SDL_GPU_STOREOP_STORE,
      },
    };
    ImGui_ImplSDLGPU3_PrepareDrawData(drawData, cmd);
    SDL_GPURenderPass *imguiRenderPass = SDL_BeginGPURenderPass(cmd, imguiColorTarget, SDL_arraysize(imguiColorTarget), nullptr);
    ImGui_ImplSDLGPU3_RenderDrawData(drawData, cmd, imguiRenderPass);
    SDL_EndGPURenderPass(imguiRenderPass);
  }

  renderFence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd);
  if (!renderFence) {
    SDLFAIL();
  }
}

void GUI::shutdown() {
  SDL_WaitForGPUIdle(gpu);
  ImGui_ImplSDL3_Shutdown();
  ImGui_ImplSDLGPU3_Shutdown();
  ImGui::DestroyContext();
  SDL_ReleaseWindowFromGPUDevice(gpu, window);
  SDL_DestroyGPUDevice(gpu);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
