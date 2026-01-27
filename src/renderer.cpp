/** @copyright 2025 Sean Kasun */

#include "renderer.h"
#include "SDL3/SDL_stdinc.h"
#include "pipelines.h"
#include "terrafirma.h"
#include <SDL3/SDL_gpu.h>
#include <memory>

static const int maxInstances = 512 * 512;
static const int maxInstanceLen = maxInstances * sizeof(float) * 10;

std::string Renderer::init(SDL_GPUDevice *gpu) {
  this->gpu = gpu;
  const auto err = pipelines.init(gpu);
  if (!err.empty()) {
    return err;
  }

  SDL_GPUTransferBufferCreateInfo transferInfo {
    .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
    .size =  maxInstanceLen,
  };
  transfer = SDL_CreateGPUTransferBuffer(gpu, &transferInfo);
  if (transfer == nullptr) {
    SDLFAIL();
  }

  SDL_GPUBufferCreateInfo tileInfo {
    .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
    .size = maxInstanceLen,
  };
  tiles = SDL_CreateGPUBuffer(gpu, &tileInfo);

  SDL_GPUSamplerCreateInfo samplerInfo {
    .min_filter = SDL_GPU_FILTER_NEAREST,
    .mag_filter = SDL_GPU_FILTER_NEAREST,
    .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
    .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
  };
  sampler = SDL_CreateGPUSampler(gpu, &samplerInfo);

  SDL_GPUSamplerCreateInfo bgSamplerInfo {
    .min_filter = SDL_GPU_FILTER_NEAREST,
    .mag_filter = SDL_GPU_FILTER_NEAREST,
    .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
    .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
    .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
    .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
  };
  bgSampler = SDL_CreateGPUSampler(gpu, &bgSamplerInfo);

  return "";
}

bool Renderer::setTextures(const std::filesystem::path &path) {
  return textures.setPath(path);
}

void Renderer::clear() {
  toDraw.clear();
  toOverlay.clear();
  tileInstances.clear();
  backgroundInstances.clear();
  liquidInstances.clear();
  flatInstances.clear();
  hiliteInstances.clear();
}

void Renderer::addGroup(int slot, Pipeline pipeline, SDL_GPUTexture *tex, SDL_GPUSampler *sampler, glm::vec2 size, float z, size_t offset) {
  std::shared_ptr<RenderData> group = nullptr;
  if (pipeline == Pipeline::Hilite || pipeline == Pipeline::Liquid) {
    group = toOverlay[slot];
  } else {
    group = toDraw[slot];
  }
  if (group == nullptr) {
    group = std::make_shared<RenderData>();
    group->pipeline = pipeline;
    group->tex = tex;
    group->sampler = sampler;
    group->layer = z;
    group->uvdims = size;
    if (pipeline == Pipeline::Hilite || pipeline == Pipeline::Liquid) {
      toOverlay[slot] = group;
    } else {
      toDraw[slot] = group;
    }
  }
  group->offsets.push_back(offset);
}

void Renderer::addTile(SDL_GPUCopyPass *copy, int slot, float x, float y, float z, int w, int h, float u, float v, uint8_t paint, bool fliph, bool flipv) {
  auto tex = textures.get(gpu, copy, slot);
  if (tex == nullptr) {
    return;
  }
  auto size = textures.size(slot);

  addGroup(slot, Pipeline::Tile, tex, sampler, size, z, tileInstances.size());

  if (w == 0) {
    w = size.x;
  }
  if (h == 0) {
    h = size.y;
  }


  uint32_t slope = 0;
  if (fliph || flipv) {
    slope = 4;
    if (fliph) {
      slope++;
    }
    if (flipv) {
      slope += 2;
    }
  }

  tileInstances.emplace_back(glm::vec2(x, y),
                             glm::vec2(w, h),
                             glm::vec2((u + 0.5f) / size.x, (v + 0.5f) / size.y),
                             paint, slope);
}

void Renderer::addSlope(SDL_GPUCopyPass *copy, int slot, int slope, float x, float y, float z, int w, int h, float u, float v, uint8_t paint) {
  auto tex = textures.get(gpu, copy, slot);
  if (tex == nullptr) {
    return;
  }
  auto size = textures.size(slot);

  addGroup(slot, Pipeline::Tile, tex, sampler, size, z, tileInstances.size());

  tileInstances.emplace_back(glm::vec2(x, y),
                             glm::vec2(w, h),
                             glm::vec2((u + 0.5f) / size.x, (v + 0.5f) / size.y),
                             paint, slope);
}

void Renderer::addHBG(SDL_GPUCopyPass *copy, int slot, float x, float y, float w, float h) {
  // special bg that only tiles horizontally
  auto tex = textures.get(gpu, copy, slot);
  if (tex == nullptr) {
    return;
  }
  auto size = textures.size(slot);
  addGroup(slot, Pipeline::Background, tex, bgSampler, size, 0.5, backgroundInstances.size());
  backgroundInstances.emplace_back(glm::vec2(x * 16, y * 16),
                                   glm::vec2(w * 16, h * 16),
                                   glm::vec2(size.x, h * 16));
}

void Renderer::addBG(SDL_GPUCopyPass *copy, int slot, float x, float y, float w, float h) {
  auto tex = textures.get(gpu, copy, slot);
  if (tex == nullptr) {
    return;
  }
  auto size = textures.size(slot);
  addGroup(slot, Pipeline::Background, tex, bgSampler, size, 0.5, backgroundInstances.size());
  backgroundInstances.emplace_back(glm::vec2(x * 16, y * 16),
                                   glm::vec2(w * 16, h * 16),
                                   size);
}

void Renderer::addLiquid(SDL_GPUCopyPass *copy, int slot, int x, int y, float z, int w, int h, float v, float alpha) {
  auto tex = textures.get(gpu, copy, slot);
  if (tex == nullptr) {
    return;
  }
  auto size = textures.size(slot);

  addGroup(slot, Pipeline::Liquid, tex, sampler, size, z, liquidInstances.size());
  liquidInstances.emplace_back(glm::vec2(x, y),
                               glm::vec2(w, h),
                               glm::vec2(0, (v + 0.5f) / size.y),
                               alpha);
}

void Renderer::addHouse(SDL_GPUCopyPass *copy, int slot, float x, float y, float z) {
  int bannerSlot = Textures::Unique | Textures::Banner;
  auto tex = textures.get(gpu, copy, bannerSlot);
  if (tex == nullptr) {
    return;
  }
  auto size = textures.size(bannerSlot);
  addGroup(bannerSlot, Pipeline::Tile, tex, sampler, size, z, tileInstances.size());
  tileInstances.emplace_back(glm::vec2(x - size.x / 2, y - size.y / 2),
                             glm::vec2(32, 40),
                             glm::vec2(0, 0),
                             0, 0);

  tex = textures.get(gpu, copy, slot);
  if (tex == nullptr) {
    return;
  }
  size = textures.size(slot);
  addGroup(slot, Pipeline::Tile, tex, sampler, size, z + 0.5, tileInstances.size());
  tileInstances.emplace_back(glm::vec2(x - size.x / 2, y - size.y / 2),
                             size,
                             glm::vec2(0, 0),
                             0, 0);
}

void Renderer::addHilite(SDL_GPUCopyPass *copy, float x, float y, float w, float h) {
  glm::vec2 size(w, h);
  addGroup(Textures::Hilite, Pipeline::Hilite, nullptr, nullptr, size, 10.0f, hiliteInstances.size());
  hiliteInstances.emplace_back(glm::vec2(x, y), size);
}

void Renderer::addFlat(SDL_GPUCopyPass *copy, void *data, float x, float y, float x2, float y2, uint32_t w, uint32_t h) {
  auto tex = textures.flat(gpu, copy, data, w, h);
  if (tex == nullptr) {
    return;
  }
  auto size = textures.size(Textures::Flat);
  addGroup(Textures::Flat, Pipeline::Flat, tex, sampler, size * 16.0f, 1.0, flatInstances.size());
  glm::vec2 dims(x2 - x, y2 - y);
  flatInstances.emplace_back(glm::vec2(x * 16, y * 16), dims * 16.f,
                             glm::vec2(x, y) / size,
                             glm::vec2(x2 - x, y2 - y) / size);
}

void Renderer::resetFlat() {
  textures.resetFlat(gpu);
}

void Renderer::copy(SDL_GPUCopyPass *copy) {
  uint8_t *buf = (uint8_t*)SDL_MapGPUTransferBuffer(gpu, transfer, true);
  uint32_t offset = 0;
  for (auto &d : toDraw) {
    offset = copyGroup(copy, buf, d.second, offset);
  }
  for (auto &d : toOverlay) {
    offset = copyGroup(copy, buf, d.second, offset);
  }
  SDL_UnmapGPUTransferBuffer(gpu, transfer);

  SDL_GPUTransferBufferLocation source {
    .transfer_buffer = transfer,
    .offset = 0,
  };

  SDL_GPUBufferRegion dest {
    .buffer = tiles,
    .offset = 0,
    .size = offset,
  };

  SDL_UploadToGPUBuffer(copy, &source, &dest, true);
}

uint32_t Renderer::copyGroup(SDL_GPUCopyPass *copy, uint8_t *buf, std::shared_ptr<RenderData> group, uint32_t offset) {
  group->offset = offset;
  uint8_t *src = nullptr;
  int blocklen = 0;
  switch (group->pipeline) {
    case Pipeline::Tile:
      src = (uint8_t*)tileInstances.data();
      blocklen = sizeof(TileInstance);
      break;
    case Pipeline::Background:
      src = (uint8_t*)backgroundInstances.data();
      blocklen = sizeof(BackgroundInstance);
      break;
    case Pipeline::Liquid:
      src = (uint8_t*)liquidInstances.data();
      blocklen = sizeof(LiquidInstance);
      break;
    case Pipeline::Flat:
      src = (uint8_t*)flatInstances.data();
      blocklen = sizeof(FlatInstance);
      break;
    case Pipeline::Hilite:
      src = (uint8_t*)hiliteInstances.data();
      blocklen = sizeof(HiliteInstance);
      break;
  }
  for (auto i : group->offsets) {
    if (offset + blocklen < maxInstanceLen) {
      SDL_memcpy(buf + offset, src + i * blocklen, blocklen);
      offset += blocklen;
    }
  }
  return offset;
}

void Renderer::render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *render, const glm::mat4 &ortho) {
  for (const auto &i: toDraw) {
    renderGroup(cmd, render, ortho, i.second);
  }
  // render transparent last
  for (const auto &i: toOverlay) {
    renderGroup(cmd, render, ortho, i.second);
  }
}

void Renderer::renderGroup(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *render, const glm::mat4 &ortho, std::shared_ptr<RenderData> group) {
  SDL_BindGPUGraphicsPipeline(render, pipelines.get(group->pipeline));
    SDL_GPUBufferBinding vertexBinding = {
    .buffer = tiles,
    .offset = group->offset,
  };

  SDL_GPUTextureSamplerBinding textureBinding = {
    .texture = group->tex,
    .sampler = group->sampler,
  };

  struct {
    glm::vec2 hiliting;
  } fub;
  fub.hiliting.x = hiliting ? 1 : 0;  // whether or not to dim everything else
  double unused;
  fub.hiliting.y = sin(SDL_GetTicks() * 3.14159 / 180.0) * 0.5 + 0.5;  // pulse
  
  struct {
    glm::mat4 ortho;
    glm::vec2 uvdims;
    float layer;
  } ub;
  ub.ortho = ortho;
  ub.uvdims = group->uvdims;
  ub.layer = group->layer;

  SDL_BindGPUVertexBuffers(render, 0, &vertexBinding, 1);
  if (group->pipeline != Pipeline::Hilite) {
    SDL_BindGPUFragmentSamplers(render, 0, &textureBinding, 1);
  }
  SDL_PushGPUVertexUniformData(cmd, 0, &ub, sizeof(ub));
  SDL_PushGPUFragmentUniformData(cmd, 0, &fub, sizeof(fub));
  SDL_DrawGPUPrimitives(render, 4, group->offsets.size(), 0, 0);
}

void Renderer::hiliteBlock(bool hilite) {
  hiliting = hilite;
}
