/** @copyright 2025 Sean Kasun */

#include "textures.h"
#include "handle.h"
#include "lzx.h"
#include "gui.h"
#include <SDL3/SDL_gpu.h>
#include <filesystem>

bool Textures::setPath(const std::filesystem::path &path) {
  cache.clear();
  dims.clear();
  root = path;
  // are there are images here?
  return std::filesystem::is_directory(path) && std::filesystem::exists(path / "Tiles_0.xnb");
}

SDL_GPUTexture *Textures::get(SDL_GPUDevice *gpu, SDL_GPUCopyPass *copy, int slot) {
  if (cache[slot] == nullptr) {
    TextureSlot mask = static_cast<TextureSlot>(slot & 0xff000);
    int num = slot & 0xfff;
    switch (mask) {
      case Textures::Tile:
        load(gpu, copy, slot, "Tiles_" + std::to_string(num));
        break;
      case Textures::Wall:
        load(gpu, copy, slot, "Wall_" + std::to_string(num));
        break;
      case Textures::ArmorHead:
        load(gpu, copy, slot, "Armor_Head_" + std::to_string(num));
        break;
      case Textures::ArmorBody:
        load(gpu, copy, slot, "Armor/Armor_" + std::to_string(num));
        break;
      case Textures::ArmorLegs:
        load(gpu, copy, slot, "Armor_Legs_" + std::to_string(num));
        break;
      case Textures::TreeTops:
        load(gpu, copy, slot, "Tree_Tops_" + std::to_string(num));
        break;
      case Textures::TreeBranches:
        load(gpu, copy, slot, "Tree_Branches_" + std::to_string(num));
        break;
      case Textures::Extra:
        load(gpu, copy, slot, "Extra_" + std::to_string(num));
        break;
      case Textures::Xmas:
        load(gpu, copy, slot, "Xmas_" + std::to_string(num));
        break;
      case Textures::Background:
        load(gpu, copy, slot, "Background_" + std::to_string(num));
        break;
      case Textures::Underworld:
        load(gpu, copy, slot, "Backgrounds/Underworld " + std::to_string(num));
        break;
      case Textures::Liquid:
      case Textures::LiquidEdge:  // this is a separate slot for z-indexing
        load(gpu, copy, slot, "Liquid_" + std::to_string(num));
        break;
      case Textures::NPC:
        load(gpu, copy, slot, "NPC_" + std::to_string(num));
        break;
      case Textures::NPCHead:
        load(gpu, copy, slot, "NPC_Head_" + std::to_string(num));
        break;
      case Textures::Unique:
        switch (num) {
          case Textures::Outline:
            load(gpu, copy, slot, "Wall_Outline");
            break;
          case Textures::Shroom:
            load(gpu, copy, slot, "Shroom_Tops");
            break;
          case Textures::Actuator:
            load(gpu, copy, slot, "Actuator");
            break;
          case Textures::Wires:
            load(gpu, copy, slot, "WiresNew");
            break;
          case Textures::Banner:
            load(gpu, copy, slot, "House_Banner_1");
            break;
        }
        break;
      default:
        FAIL("missing texture");
    }
  }
  return cache[slot];
}

glm::vec2 Textures::size(int slot) {
  return dims[slot];
}

SDL_GPUTexture *Textures::flat(SDL_GPUDevice *gpu, SDL_GPUCopyPass *copy, void *data, uint32_t w, uint32_t h) {
  auto tex = cache[Flat];
  if (tex) {
    return tex;
  }
  SDL_GPUTextureCreateInfo info {
    .type = SDL_GPU_TEXTURETYPE_2D,
    .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
    .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
    .width = w,
    .height = h,
    .layer_count_or_depth = 1,
    .num_levels = 1,
  };
  tex = SDL_CreateGPUTexture(gpu, &info);
  cache[Flat] = tex;
  dims[Flat] = glm::vec2(w, h);
  uint32_t len = w * h * 4;
  SDL_GPUTransferBufferCreateInfo transferCreateInfo {
    .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
    .size = len,
  };
  SDL_GPUTransferBuffer *transfer = SDL_CreateGPUTransferBuffer(gpu, &transferCreateInfo);
  void *dest = SDL_MapGPUTransferBuffer(gpu, transfer, true);
  SDL_memcpy(dest, data, len);
  SDL_UnmapGPUTransferBuffer(gpu, transfer);

  SDL_GPUTextureTransferInfo transferInfo {
    .transfer_buffer = transfer,
    .offset = 0,
  };
  SDL_GPUTextureRegion region {
    .texture = tex,
    .w = w,
    .h = h,
    .d = 1,
  };
  SDL_UploadToGPUTexture(copy, &transferInfo, &region, true);
  SDL_ReleaseGPUTransferBuffer(gpu, transfer);
  return tex;
}

void Textures::resetFlat(SDL_GPUDevice *gpu) {
  SDL_ReleaseGPUTexture(gpu, cache[Flat]);
  cache[Flat] = nullptr;
}

void Textures::load(SDL_GPUDevice *gpu, SDL_GPUCopyPass *copy, int slot, const std::string name) {
  auto path = root / (name + ".xnb");
  Handle handle(path.string());
  if (!handle.isOpen()) {
    SDL_Log("Failed to open texture: %s", path.string().c_str());
    return;  // ignore missing textures
  }

  auto header = handle.r32();
  if (header != 0x77424e58 && header != 0x78424e58 && header != 0x6d424e58) {
    FAIL("Not a valid XNB");
  }

  auto version = handle.r16();
  bool compressed = version & 0x8000;
  version &= 0xff;
  if (version != 4 && version != 5) {
    FAIL("Invalid XNB version");
  }

  bool rawAllocated = false;
  uint8_t *raw = nullptr;

  auto length = handle.r32();
  if (compressed) {
    auto decompLength = handle.r32();
    uint8_t *p = handle.readBytes(length - 4);
    uint8_t *endp = p + length - 4;
    length = decompLength;
    raw = new uint8_t[length];
    rawAllocated = true;
    uint8_t *dp = raw;
    struct LZXstate *lzx = LZXinit(16);
    while (p < endp) {
      uint8_t hi = *p++;
      uint8_t lo = *p++;
      uint16_t compLen = (hi << 8) | lo;
      uint16_t decompLen = 0x8000;
      if (hi == 0xff) {
        hi = lo;
        lo = *p++;
        decompLen = (hi << 8) | lo;
        hi = *p++;
        lo = *p++;
        compLen = (hi << 8) | lo;
      }
      if (compLen == 0 || decompLen == 0) {  // done
        break;
      }
      LZXdecompress(lzx, p, dp, compLen, decompLen);
      p += compLen;
      dp += decompLen;
    }
    LZXteardown(lzx);
  } else {
    raw = handle.readBytes(length);
  }

  Handle tex(raw, length);

  int numReaders = 0;
  int bits = 0;
  uint8_t b7;
  do {
    b7 = tex.r8();
    numReaders |= (b7 & 0x7f) << bits;
    bits += 7;
  } while (b7 & 0x80);
  for (int i = 0; i < numReaders; i++) {
    tex.rs();  // name of reader
    tex.r32();  // version
  }

  while (tex.r8() & 0x80) {}  // skip # shared res
  while (tex.r8() & 0x80) {}  // skip type id

  int format = tex.r32();
  auto width = tex.r32();
  auto height = tex.r32();
  tex.r32();  // mipmap
  tex.r32();  // image length

  SDL_GPUTextureCreateInfo info {
    .type = SDL_GPU_TEXTURETYPE_2D,
    .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
    .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
    .width = width,
    .height = height,
    .layer_count_or_depth = 1,
    .num_levels = 1,
  };

  auto texture = SDL_CreateGPUTexture(gpu, &info);

  SDL_GPUTransferBufferCreateInfo transferCreateInfo {
    .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
    .size = info.width * info.height * 4
  };

  SDL_GPUTransferBuffer *transfer = SDL_CreateGPUTransferBuffer(gpu, &transferCreateInfo);
  uint8_t *data = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(gpu, transfer, true));
  if (format != 0) {  // bgra32
    FAIL("Invalid format");
  }
  SDL_memcpy(data, tex.readBytes(width * height * 4), width * height * 4);
  SDL_UnmapGPUTransferBuffer(gpu, transfer);

  SDL_GPUTextureTransferInfo transferInfo {
    .transfer_buffer = transfer,
    .offset = 0,
  };
  SDL_GPUTextureRegion region {
    .texture = texture,
    .w = info.width,
    .h = info.height,
    .d = 1,
  };
  SDL_UploadToGPUTexture(copy, &transferInfo, &region, true);
  SDL_ReleaseGPUTransferBuffer(gpu, transfer);
  
  if (rawAllocated) {
    delete []raw;
  }
  dims[slot] = glm::vec2(info.width, info.height);
  cache[slot] = texture;
}
