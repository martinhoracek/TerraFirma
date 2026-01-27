/** @copyright 2025 Sean Kasun */

#include "pipelines.h"
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_stdinc.h"
#include "shaders.h"

static ShaderSource shaderSources[] = {
  // tiles fragment
  {
    tiles_frag_spv, tiles_frag_msl, tiles_frag_dxil,
    tiles_frag_spv_length, tiles_frag_msl_length, tiles_frag_dxil_length,
  },
  // tiles vertex
  {
    tiles_vert_spv, tiles_vert_msl, tiles_vert_dxil,
    tiles_vert_spv_length, tiles_vert_msl_length, tiles_vert_dxil_length,
  },
  // background fragment
  {
    background_frag_spv, background_frag_msl, background_frag_dxil,
    background_frag_spv_length, background_frag_msl_length, background_frag_dxil_length,
  },
  // background vertex
  {
    background_vert_spv, background_vert_msl, background_vert_dxil,
    background_vert_spv_length, background_vert_msl_length, background_vert_dxil_length,
  },
  // water fragment
  {
    liquid_frag_spv, liquid_frag_msl, liquid_frag_dxil,
    liquid_frag_spv_length, liquid_frag_msl_length, liquid_frag_dxil_length,
  },
  // liquid vertex
  {
    liquid_vert_spv, liquid_vert_msl, liquid_vert_dxil,
    liquid_vert_spv_length, liquid_vert_msl_length, liquid_vert_dxil_length,
  },
  // flat vertex
  {
    flat_vert_spv, flat_vert_msl, flat_vert_dxil,
    flat_vert_spv_length, flat_vert_msl_length, flat_vert_dxil_length,  
  },
  // hilite fragment
  {
    hilite_frag_spv, hilite_frag_msl, hilite_frag_dxil,
    hilite_frag_spv_length, hilite_frag_msl_length, hilite_frag_dxil_length,
  },
  // hilite vertex
  {
    hilite_vert_spv, hilite_vert_msl, hilite_vert_dxil,
    hilite_vert_spv_length, hilite_vert_msl_length, hilite_vert_dxil_length,
  }
};

std::string Pipelines::init(SDL_GPUDevice *gpu) {
  // Tiles
  SDL_GPUColorTargetBlendState opaqueColor = {
    .enable_blend = false,
  };
  SDL_GPUColorTargetDescription colorTarget = {
    .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB,
    .blend_state = opaqueColor,
  };
  SDL_GPUShaderCreateInfo fragInfo = {
    .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
    .num_samplers = 1,
    .num_uniform_buffers = 1,
  };
  auto fShader = loadShader(gpu, shaderSources[0], fragInfo);
  if (!fShader) {
    return "Tile fragment shader failed";
  }
  SDL_GPUShaderCreateInfo vertInfo = {
    .stage = SDL_GPU_SHADERSTAGE_VERTEX,
    .num_uniform_buffers = 1,
  };
  // Tiles
  auto vShader = loadShader(gpu, shaderSources[1], vertInfo);
  if (!vShader) {
    return "Tile vertex shader failed";
  }
  SDL_GPUVertexBufferDescription tileVertexBuffers[] = {
    {
      .slot = 0,
      .pitch = sizeof(float) * 6 + sizeof(uint32_t) * 2,
      .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE,
    }
  };
  SDL_GPUVertexAttribute tileVertexAttrs[] = {
    {  // translate
      .location = 0,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
      .offset = 0,
    }, {  // size
      .location = 1,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
      .offset = sizeof(float) * 2,
    }, {  // uv
      .location = 2,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
      .offset = sizeof(float) * 4,
    }, {  // paint
      .location = 3,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_INT,
      .offset = sizeof(float) * 6,
    }, {  // slope
      .location = 4,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_INT,
      .offset = sizeof(float) * 6 + sizeof(uint32_t),
    },
  };
  SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {
    .vertex_shader = vShader,
    .fragment_shader = fShader,
    .vertex_input_state = {
      .vertex_buffer_descriptions = tileVertexBuffers,
      .num_vertex_buffers = SDL_arraysize(tileVertexBuffers),
      .vertex_attributes = tileVertexAttrs,
      .num_vertex_attributes = SDL_arraysize(tileVertexAttrs),
    },
    .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP,
    .rasterizer_state = {
      .fill_mode = SDL_GPU_FILLMODE_FILL,
      .cull_mode = SDL_GPU_CULLMODE_NONE,  // triangle strips don't have a winding
    },
    .depth_stencil_state = {
      .compare_op = SDL_GPU_COMPAREOP_GREATER,
      .compare_mask = 0xff,
      .write_mask = 0xff,
      .enable_depth_test = true,
      .enable_depth_write = true,
      .enable_stencil_test = false,
    },
    .target_info = {
      .color_target_descriptions = &colorTarget,
      .num_color_targets = 1,
      .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
      .has_depth_stencil_target = true,
    },
  };
  pipelines[Pipeline::Tile] = SDL_CreateGPUGraphicsPipeline(gpu, &pipelineInfo);
  SDL_ReleaseGPUShader(gpu, vShader);
  SDL_ReleaseGPUShader(gpu, fShader);

  // Background
  fShader = loadShader(gpu, shaderSources[2], fragInfo);
  if (!fShader) {
    return "Background fragment shader failed";
  }
  vShader = loadShader(gpu, shaderSources[3], vertInfo);
  if (!vShader) {
    return "Backgorund vertex shader failed";
  }
  SDL_GPUVertexBufferDescription bgVertexBuffers[] = {
    {
      .slot = 0,
      .pitch = sizeof(float) * 6,
      .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE,
    },
  };
  SDL_GPUVertexAttribute bgVertexAttrs[] = {
    {  // translate
      .location = 0,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
      .offset = 0,
    }, {  // size
      .location = 1,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
      .offset = sizeof(float) * 2,
    }, {  // uv
      .location = 2,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
      .offset = sizeof(float) * 4,
    },
  };
  pipelineInfo.vertex_shader = vShader;
  pipelineInfo.fragment_shader = fShader;
  pipelineInfo.vertex_input_state.vertex_buffer_descriptions = bgVertexBuffers;
  pipelineInfo.vertex_input_state.num_vertex_buffers = SDL_arraysize(bgVertexBuffers);
  pipelineInfo.vertex_input_state.vertex_attributes = bgVertexAttrs;
  pipelineInfo.vertex_input_state.num_vertex_attributes = SDL_arraysize(bgVertexAttrs);
  pipelines[Pipeline::Background] = SDL_CreateGPUGraphicsPipeline(gpu, &pipelineInfo);
  SDL_ReleaseGPUShader(gpu, vShader);
  // don't release frag shader yet, flat uses it

  // Flat
  vShader = loadShader(gpu, shaderSources[6], vertInfo);
  if (!vShader) {
    return "Backgorund vertex shader failed";
  }
  SDL_GPUVertexBufferDescription flatVertexBuffers[] = {
    {
      .slot = 0,
      .pitch = sizeof(float) * 8,
      .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE,
    },
  };
  SDL_GPUVertexAttribute flatVertexAttrs[] = {
    {  // translate
      .location = 0,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
      .offset = 0,
    }, {  // size
      .location = 1,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
      .offset = sizeof(float) * 2,
    }, {  // uv
      .location = 2,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
      .offset = sizeof(float) * 4,
    }, {  // uvsize
      .location = 3,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
      .offset = sizeof(float) * 6,
    },
  };
  pipelineInfo.vertex_shader = vShader;
  pipelineInfo.fragment_shader = fShader;
  pipelineInfo.vertex_input_state.vertex_buffer_descriptions = flatVertexBuffers;
  pipelineInfo.vertex_input_state.num_vertex_buffers = SDL_arraysize(flatVertexBuffers);
  pipelineInfo.vertex_input_state.vertex_attributes = flatVertexAttrs;
  pipelineInfo.vertex_input_state.num_vertex_attributes = SDL_arraysize(flatVertexAttrs);
  pipelines[Pipeline::Flat] = SDL_CreateGPUGraphicsPipeline(gpu, &pipelineInfo);
  SDL_ReleaseGPUShader(gpu, vShader);
  SDL_ReleaseGPUShader(gpu, fShader);


  // Liquid and hilites are last because of transparency
  SDL_GPUColorTargetBlendState transColor = {
    .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
    .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
    .color_blend_op = SDL_GPU_BLENDOP_ADD,
    .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
    .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO,
    .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
    .color_write_mask =
      SDL_GPU_COLORCOMPONENT_R | SDL_GPU_COLORCOMPONENT_G |
      SDL_GPU_COLORCOMPONENT_B | SDL_GPU_COLORCOMPONENT_A,
    .enable_blend = true,
  };
  colorTarget.blend_state = transColor;
  fShader = loadShader(gpu, shaderSources[4], fragInfo);
  if (!fShader) {
    return "liquid fragment shader failed";
  }
  vShader = loadShader(gpu, shaderSources[5], vertInfo);
  if (!vShader) {
    return "liquid vertex shader failed";
  }
  SDL_GPUVertexBufferDescription liquidVertexBuffers[] = {
    {
      .slot = 0,
      .pitch = sizeof(float) * 7,
      .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE,
    },
  };
  SDL_GPUVertexAttribute liquidVertexAttrs[] = {
    {  // translate
      .location = 0,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
      .offset = 0,
    }, { //  size
      .location = 1,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
      .offset = sizeof(float) * 2,
    }, { // uv
      .location = 2,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
      .offset = sizeof(float) * 4,
    }, {  // alpha
      .location = 3,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
      .offset = sizeof(float) * 6,
    },
  };
  pipelineInfo.vertex_shader = vShader;
  pipelineInfo.fragment_shader = fShader;
  pipelineInfo.vertex_input_state.vertex_buffer_descriptions = liquidVertexBuffers;
  pipelineInfo.vertex_input_state.num_vertex_buffers = SDL_arraysize(liquidVertexBuffers);
  pipelineInfo.vertex_input_state.vertex_attributes = liquidVertexAttrs;
  pipelineInfo.vertex_input_state.num_vertex_attributes = SDL_arraysize(liquidVertexAttrs);
  pipelines[Pipeline::Liquid] = SDL_CreateGPUGraphicsPipeline(gpu, &pipelineInfo);
  SDL_ReleaseGPUShader(gpu, vShader);
  SDL_ReleaseGPUShader(gpu, fShader);

  // Hilite
  fShader = loadShader(gpu, shaderSources[7], fragInfo);
  if (!fShader) {
    return "hilite fragment shader failed";
  }
  vShader = loadShader(gpu, shaderSources[8], vertInfo);
  if (!vShader) {
    return "hilite vertex shader failed";
  }
  SDL_GPUVertexBufferDescription hiliteVertexBuffers[] = {
    {
      .slot = 0,
      .pitch = sizeof(float) * 4,
      .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE,
    },
  };
  SDL_GPUVertexAttribute hiliteVertexAttrs[] = {
    {  // translate
      .location = 0,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
      .offset = 0,
    }, {  // size
      .location = 1,
      .buffer_slot = 0,
      .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
      .offset = sizeof(float) * 2,
    },
  };
  pipelineInfo.vertex_shader = vShader;
  pipelineInfo.fragment_shader = fShader;
  pipelineInfo.vertex_input_state.vertex_buffer_descriptions = hiliteVertexBuffers;
  pipelineInfo.vertex_input_state.num_vertex_buffers = SDL_arraysize(hiliteVertexBuffers);
  pipelineInfo.vertex_input_state.vertex_attributes = hiliteVertexAttrs;
  pipelineInfo.vertex_input_state.num_vertex_attributes = SDL_arraysize(hiliteVertexAttrs);
  pipelines[Pipeline::Hilite] = SDL_CreateGPUGraphicsPipeline(gpu, &pipelineInfo);
  SDL_ReleaseGPUShader(gpu, vShader);
  SDL_ReleaseGPUShader(gpu, fShader);
  
  return "";
}

SDL_GPUGraphicsPipeline *Pipelines::get(Pipeline p) {
  return pipelines.at(p);
}

SDL_GPUShader *Pipelines::loadShader(SDL_GPUDevice *gpu, const ShaderSource &source,
                                     const SDL_GPUShaderCreateInfo &createInfo) {
  SDL_GPUShaderFormat avail = SDL_GetGPUShaderFormats(gpu);
  SDL_GPUShaderCreateInfo info = createInfo;
  if (avail & SDL_GPU_SHADERFORMAT_SPIRV) {
    info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    info.entrypoint = "main";
    info.code = source.spv;
    info.code_size = source.spvSize;
  } else if (avail & SDL_GPU_SHADERFORMAT_MSL) {
    info.format = SDL_GPU_SHADERFORMAT_MSL;
    info.entrypoint = "main0";
    info.code = source.msl;
    info.code_size = source.mslSize;
  } else if (avail & SDL_GPU_SHADERFORMAT_DXIL) {
    info.format = SDL_GPU_SHADERFORMAT_DXIL;
    info.entrypoint = "main";
    info.code = source.dxil;
    info.code_size = source.dxilSize;
  } else {
    return nullptr;
  }

  return SDL_CreateGPUShader(gpu, &info);
}
