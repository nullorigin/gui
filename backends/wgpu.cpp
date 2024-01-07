// gui: Renderer for WebGPU
// This needs to be used along with a Platform Binding (e.g. GLFW)
// (Please note that WebGPU is currently experimental, will not run on non-beta
// browsers, and may break.)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'WGPUTextureView' as TextureID.
//  Read the FAQ about TextureID! [X] Renderer: Large meshes support (64k+
//  vertices) with 16-bit indices.
// Missing features:
//  [ ] Renderer: Multi-viewport support (multiple windows). Not meaningful on
//  the web.

#include "../gui.hpp"
#ifndef DISABLE
#include "wgpu.hpp"
#include <limits.h>
#include <webgpu/webgpu.h>

// Gui prototypes from internal.hpp
extern int HashData(const void *data_p, size_t data_size,
                    unsigned int seed = 0);
#define MEMALIGN(_SIZE, _ALIGN)                                                \
  (((_SIZE) + ((_ALIGN)-1)) &                                                  \
   ~((_ALIGN)-1)) // Memory align (copied from ALIGN() macro).

// WebGPU data
struct RenderResources {
  WGPUTexture FontTexture = nullptr;         // Font texture
  WGPUTextureView FontTextureView = nullptr; // Texture view for font texture
  WGPUSampler Sampler = nullptr;             // Sampler for the font texture
  WGPUBuffer Uniforms = nullptr;             // Shader uniforms
  WGPUBindGroup CommonBindGroup =
      nullptr; // Resources bind-group to bind the common resources to pipeline
  Storage ImageBindGroups; // Resources bind-group to bind the font/image
                           // resources to pipeline (this is a key->value map)
  WGPUBindGroup ImageBindGroup = nullptr; // Default font-resource of Gui
  WGPUBindGroupLayout ImageBindGroupLayout =
      nullptr; // Cache layout used for the image bind group. Avoids allocating
               // unnecessary JS objects when working with WebASM
};

struct FrameResources {
  WGPUBuffer IndexBuffer;
  WGPUBuffer VertexBuffer;
  DrawIdx *IndexBufferHost;
  DrawVert *VertexBufferHost;
  int IndexBufferSize;
  int VertexBufferSize;
};

struct Uniforms {
  float MVP[4][4];
  float Gamma;
};

struct WGPU_Data {
  WGPUDevice wgpuDevice = nullptr;
  WGPUQueue defaultQueue = nullptr;
  WGPUTextureFormat renderTargetFormat = WGPUTextureFormat_Undefined;
  WGPUTextureFormat depthStencilFormat = WGPUTextureFormat_Undefined;
  WGPURenderPipeline pipelineState = nullptr;

  RenderResources renderResources;
  FrameResources *pFrameResources = nullptr;
  unsigned int numFramesInFlight = 0;
  unsigned int frameIndex = UINT_MAX;
};

// Backend data stored in io.BackendRendererUserData to allow support for
// multiple Gui contexts It is STRONGLY preferred that you use docking
// branch with multi-viewports (== single Gui context + multiple windows)
// instead of multiple Gui contexts.
static WGPU_Data *WGPU_GetBackendData() {
  return Gui::GetCurrentContext()
             ? (WGPU_Data *)Gui::GetIO().BackendRendererUserData
             : nullptr;
}

//-----------------------------------------------------------------------------
// SHADERS
//-----------------------------------------------------------------------------

static const char __shader_vert_wgsl[] = R"(
struct VertexInput {
    @location(0) position: vec2<f32>,
    @location(1) uv: vec2<f32>,
    @location(2) color: vec4<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) uv: vec2<f32>,
};

struct Uniforms {
    mvp: mat4x4<f32>,
    gamma: f32,
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

@vertex
fn main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = uniforms.mvp * vec4<f32>(in.position, 0.0, 1.0);
    out.color = in.color;
    out.uv = in.uv;
    return out;
}
)";

static const char __shader_frag_wgsl[] = R"(
struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) uv: vec2<f32>,
};

struct Uniforms {
    mvp: mat4x4<f32>,
    gamma: f32,
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;
@group(0) @binding(1) var s: sampler;
@group(1) @binding(0) var t: texture_2d<f32>;

@fragment
fn main(in: VertexOutput) -> @location(0) vec4<f32> {
    let color = in.color * textureSample(t, s, in.uv);
    let corrected_color = pow(color.rgb, vec3<f32>(uniforms.gamma));
    return vec4<f32>(corrected_color, color.a);
}
)";

static void SafeRelease(DrawIdx *&res) {
  if (res)
    delete[] res;
  res = nullptr;
}
static void SafeRelease(DrawVert *&res) {
  if (res)
    delete[] res;
  res = nullptr;
}
static void SafeRelease(WGPUBindGroupLayout &res) {
  if (res)
    wgpuBindGroupLayoutRelease(res);
  res = nullptr;
}
static void SafeRelease(WGPUBindGroup &res) {
  if (res)
    wgpuBindGroupRelease(res);
  res = nullptr;
}
static void SafeRelease(WGPUBuffer &res) {
  if (res)
    wgpuBufferRelease(res);
  res = nullptr;
}
static void SafeRelease(WGPURenderPipeline &res) {
  if (res)
    wgpuRenderPipelineRelease(res);
  res = nullptr;
}
static void SafeRelease(WGPUSampler &res) {
  if (res)
    wgpuSamplerRelease(res);
  res = nullptr;
}
static void SafeRelease(WGPUShaderModule &res) {
  if (res)
    wgpuShaderModuleRelease(res);
  res = nullptr;
}
static void SafeRelease(WGPUTextureView &res) {
  if (res)
    wgpuTextureViewRelease(res);
  res = nullptr;
}
static void SafeRelease(WGPUTexture &res) {
  if (res)
    wgpuTextureRelease(res);
  res = nullptr;
}

static void SafeRelease(RenderResources &res) {
  SafeRelease(res.FontTexture);
  SafeRelease(res.FontTextureView);
  SafeRelease(res.Sampler);
  SafeRelease(res.Uniforms);
  SafeRelease(res.CommonBindGroup);
  SafeRelease(res.ImageBindGroup);
  SafeRelease(res.ImageBindGroupLayout);
};

static void SafeRelease(FrameResources &res) {
  SafeRelease(res.IndexBuffer);
  SafeRelease(res.VertexBuffer);
  SafeRelease(res.IndexBufferHost);
  SafeRelease(res.VertexBufferHost);
}

static WGPUProgrammableStageDescriptor
WGPU_CreateShaderModule(const char *wgsl_source) {
  WGPU_Data *bd = WGPU_GetBackendData();

  WGPUShaderModuleWGSLDescriptor wgsl_desc = {};
  wgsl_desc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
  wgsl_desc.code = wgsl_source;

  WGPUShaderModuleDescriptor desc = {};
  desc.nextInChain = reinterpret_cast<WGPUChainedStruct *>(&wgsl_desc);

  WGPUProgrammableStageDescriptor stage_desc = {};
  stage_desc.module = wgpuDeviceCreateShaderModule(bd->wgpuDevice, &desc);
  stage_desc.entryPoint = "main";
  return stage_desc;
}

static WGPUBindGroup WGPU_CreateImageBindGroup(WGPUBindGroupLayout layout,
                                               WGPUTextureView texture) {
  WGPU_Data *bd = WGPU_GetBackendData();
  WGPUBindGroupEntry image_bg_entries[] = {{nullptr, 0, 0, 0, 0, 0, texture}};

  WGPUBindGroupDescriptor image_bg_descriptor = {};
  image_bg_descriptor.layout = layout;
  image_bg_descriptor.entryCount =
      sizeof(image_bg_entries) / sizeof(WGPUBindGroupEntry);
  image_bg_descriptor.entries = image_bg_entries;
  return wgpuDeviceCreateBindGroup(bd->wgpuDevice, &image_bg_descriptor);
}

static void WGPU_SetupRenderState(DrawData *draw_data,
                                  WGPURenderPassEncoder ctx,
                                  FrameResources *fr) {
  WGPU_Data *bd = WGPU_GetBackendData();

  // Setup orthographic projection matrix into our constant buffer
  // Our visible imgui space lies from draw_data->DisplayPos (top left) to
  // draw_data->DisplayPos+data_data->DisplaySize (bottom right).
  {
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    float mvp[4][4] = {
        {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
        {0.0f, 0.0f, 0.5f, 0.0f},
        {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
    };
    wgpuQueueWriteBuffer(bd->defaultQueue, bd->renderResources.Uniforms,
                         offsetof(Uniforms, MVP), mvp, sizeof(Uniforms::MVP));
    float gamma;
    switch (bd->renderTargetFormat) {
    case WGPUTextureFormat_ASTC10x10UnormSrgb:
    case WGPUTextureFormat_ASTC10x5UnormSrgb:
    case WGPUTextureFormat_ASTC10x6UnormSrgb:
    case WGPUTextureFormat_ASTC10x8UnormSrgb:
    case WGPUTextureFormat_ASTC12x10UnormSrgb:
    case WGPUTextureFormat_ASTC12x12UnormSrgb:
    case WGPUTextureFormat_ASTC4x4UnormSrgb:
    case WGPUTextureFormat_ASTC5x5UnormSrgb:
    case WGPUTextureFormat_ASTC6x5UnormSrgb:
    case WGPUTextureFormat_ASTC6x6UnormSrgb:
    case WGPUTextureFormat_ASTC8x5UnormSrgb:
    case WGPUTextureFormat_ASTC8x6UnormSrgb:
    case WGPUTextureFormat_ASTC8x8UnormSrgb:
    case WGPUTextureFormat_BC1RGBAUnormSrgb:
    case WGPUTextureFormat_BC2RGBAUnormSrgb:
    case WGPUTextureFormat_BC3RGBAUnormSrgb:
    case WGPUTextureFormat_BC7RGBAUnormSrgb:
    case WGPUTextureFormat_BGRA8UnormSrgb:
    case WGPUTextureFormat_ETC2RGB8A1UnormSrgb:
    case WGPUTextureFormat_ETC2RGB8UnormSrgb:
    case WGPUTextureFormat_ETC2RGBA8UnormSrgb:
    case WGPUTextureFormat_RGBA8UnormSrgb:
      gamma = 2.2f;
      break;
    default:
      gamma = 1.0f;
    }
    wgpuQueueWriteBuffer(bd->defaultQueue, bd->renderResources.Uniforms,
                         offsetof(Uniforms, Gamma), &gamma,
                         sizeof(Uniforms::Gamma));
  }

  // Setup viewport
  wgpuRenderPassEncoderSetViewport(
      ctx, 0, 0, draw_data->FramebufferScale.x * draw_data->DisplaySize.x,
      draw_data->FramebufferScale.y * draw_data->DisplaySize.y, 0, 1);

  // Bind shader and vertex buffers
  wgpuRenderPassEncoderSetVertexBuffer(ctx, 0, fr->VertexBuffer, 0,
                                       fr->VertexBufferSize * sizeof(DrawVert));
  wgpuRenderPassEncoderSetIndexBuffer(
      ctx, fr->IndexBuffer,
      sizeof(DrawIdx) == 2 ? WGPUIndexFormat_Uint16 : WGPUIndexFormat_Uint32, 0,
      fr->IndexBufferSize * sizeof(DrawIdx));
  wgpuRenderPassEncoderSetPipeline(ctx, bd->pipelineState);
  wgpuRenderPassEncoderSetBindGroup(ctx, 0, bd->renderResources.CommonBindGroup,
                                    0, nullptr);

  // Setup blend factor
  WGPUColor blend_color = {0.f, 0.f, 0.f, 0.f};
  wgpuRenderPassEncoderSetBlendConstant(ctx, &blend_color);
}

// Render function
// (this used to be set in io.RenderDrawListsFn and called by Gui::Render(),
// but you can now call this directly from your main loop)
void WGPU_RenderDrawData(DrawData *draw_data,
                         WGPURenderPassEncoder pass_encoder) {
  // Avoid rendering when minimized
  int fb_width =
      (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
  int fb_height =
      (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
  if (fb_width <= 0 || fb_height <= 0 || draw_data->CmdListsCount == 0)
    return;

  // FIXME: Assuming that this only gets called once per frame!
  // If not, we can't just re-allocate the IB or VB, we'll have to do a proper
  // allocator.
  WGPU_Data *bd = WGPU_GetBackendData();
  bd->frameIndex = bd->frameIndex + 1;
  FrameResources *fr =
      &bd->pFrameResources[bd->frameIndex % bd->numFramesInFlight];

  // Create and grow vertex/index buffers if needed
  if (fr->VertexBuffer == nullptr ||
      fr->VertexBufferSize < draw_data->TotalVtxCount) {
    if (fr->VertexBuffer) {
      wgpuBufferDestroy(fr->VertexBuffer);
      wgpuBufferRelease(fr->VertexBuffer);
    }
    SafeRelease(fr->VertexBufferHost);
    fr->VertexBufferSize = draw_data->TotalVtxCount + 5000;

    WGPUBufferDescriptor vb_desc = {
        nullptr, "Gui Vertex buffer",
        WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex,
        MEMALIGN(fr->VertexBufferSize * sizeof(DrawVert), 4), false};
    fr->VertexBuffer = wgpuDeviceCreateBuffer(bd->wgpuDevice, &vb_desc);
    if (!fr->VertexBuffer)
      return;

    fr->VertexBufferHost = new DrawVert[fr->VertexBufferSize];
  }
  if (fr->IndexBuffer == nullptr ||
      fr->IndexBufferSize < draw_data->TotalIdxCount) {
    if (fr->IndexBuffer) {
      wgpuBufferDestroy(fr->IndexBuffer);
      wgpuBufferRelease(fr->IndexBuffer);
    }
    SafeRelease(fr->IndexBufferHost);
    fr->IndexBufferSize = draw_data->TotalIdxCount + 10000;

    WGPUBufferDescriptor ib_desc = {
        nullptr, "Gui Index buffer",
        WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index,
        MEMALIGN(fr->IndexBufferSize * sizeof(DrawIdx), 4), false};
    fr->IndexBuffer = wgpuDeviceCreateBuffer(bd->wgpuDevice, &ib_desc);
    if (!fr->IndexBuffer)
      return;

    fr->IndexBufferHost = new DrawIdx[fr->IndexBufferSize];
  }

  // Upload vertex/index data into a single contiguous GPU buffer
  DrawVert *vtx_dst = (DrawVert *)fr->VertexBufferHost;
  DrawIdx *idx_dst = (DrawIdx *)fr->IndexBufferHost;
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const DrawList *cmd_list = draw_data->CmdLists[n];
    memcpy(vtx_dst, cmd_list->VtxBuffer.Data,
           cmd_list->VtxBuffer.Size * sizeof(DrawVert));
    memcpy(idx_dst, cmd_list->IdxBuffer.Data,
           cmd_list->IdxBuffer.Size * sizeof(DrawIdx));
    vtx_dst += cmd_list->VtxBuffer.Size;
    idx_dst += cmd_list->IdxBuffer.Size;
  }
  int64_t vb_write_size =
      MEMALIGN((char *)vtx_dst - (char *)fr->VertexBufferHost, 4);
  int64_t ib_write_size =
      MEMALIGN((char *)idx_dst - (char *)fr->IndexBufferHost, 4);
  wgpuQueueWriteBuffer(bd->defaultQueue, fr->VertexBuffer, 0,
                       fr->VertexBufferHost, vb_write_size);
  wgpuQueueWriteBuffer(bd->defaultQueue, fr->IndexBuffer, 0,
                       fr->IndexBufferHost, ib_write_size);

  // Setup desired render state
  WGPU_SetupRenderState(draw_data, pass_encoder, fr);

  // Render command lists
  // (Because we merged all buffers into a single one, we maintain our own
  // offset into them)
  int global_vtx_offset = 0;
  int global_idx_offset = 0;
  Vec2 clip_scale = draw_data->FramebufferScale;
  Vec2 clip_off = draw_data->DisplayPos;
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const DrawList *cmd_list = draw_data->CmdLists[n];
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const DrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback != nullptr) {
        // User callback, registered via DrawList::AddCallback()
        // (DrawCallback_ResetRenderState is a special callback value used by
        // the user to request the renderer to reset render state.)
        if (pcmd->UserCallback == DrawCallback_ResetRenderState)
          WGPU_SetupRenderState(draw_data, pass_encoder, fr);
        else
          pcmd->UserCallback(cmd_list, pcmd);
      } else {
        // Bind custom texture
        TextureID tex_id = pcmd->GetTexID();
        int tex_id_hash = HashData(&tex_id, sizeof(tex_id));
        auto bind_group =
            bd->renderResources.ImageBindGroups.GetVoidPtr(tex_id_hash);
        if (bind_group) {
          wgpuRenderPassEncoderSetBindGroup(
              pass_encoder, 1, (WGPUBindGroup)bind_group, 0, nullptr);
        } else {
          WGPUBindGroup image_bind_group = WGPU_CreateImageBindGroup(
              bd->renderResources.ImageBindGroupLayout,
              (WGPUTextureView)tex_id);
          bd->renderResources.ImageBindGroups.SetVoidPtr(tex_id_hash,
                                                         image_bind_group);
          wgpuRenderPassEncoderSetBindGroup(pass_encoder, 1, image_bind_group,
                                            0, nullptr);
        }

        // Project scissor/clipping rectangles into framebuffer space
        Vec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                      (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
        Vec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                      (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

        // Clamp to viewport as wgpuRenderPassEncoderSetScissorRect() won't
        // accept values that are off bounds
        if (clip_min.x < 0.0f) {
          clip_min.x = 0.0f;
        }
        if (clip_min.y < 0.0f) {
          clip_min.y = 0.0f;
        }
        if (clip_max.x > fb_width) {
          clip_max.x = (float)fb_width;
        }
        if (clip_max.y > fb_height) {
          clip_max.y = (float)fb_height;
        }
        if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
          continue;

        // Apply scissor/clipping rectangle, Draw
        wgpuRenderPassEncoderSetScissorRect(
            pass_encoder, (uint32_t)clip_min.x, (uint32_t)clip_min.y,
            (uint32_t)(clip_max.x - clip_min.x),
            (uint32_t)(clip_max.y - clip_min.y));
        wgpuRenderPassEncoderDrawIndexed(pass_encoder, pcmd->ElemCount, 1,
                                         pcmd->IdxOffset + global_idx_offset,
                                         pcmd->VtxOffset + global_vtx_offset,
                                         0);
      }
    }
    global_idx_offset += cmd_list->IdxBuffer.Size;
    global_vtx_offset += cmd_list->VtxBuffer.Size;
  }
}

static void WGPU_CreateFontsTexture() {
  // Build texture atlas
  WGPU_Data *bd = WGPU_GetBackendData();
  IO &io = Gui::GetIO();
  unsigned char *pixels;
  int width, height, size_pp;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &size_pp);

  // Upload texture to graphics system
  {
    WGPUTextureDescriptor tex_desc = {};
    tex_desc.label = "Gui Font Texture";
    tex_desc.dimension = WGPUTextureDimension_2D;
    tex_desc.size.width = width;
    tex_desc.size.height = height;
    tex_desc.size.depthOrArrayLayers = 1;
    tex_desc.sampleCount = 1;
    tex_desc.format = WGPUTextureFormat_RGBA8Unorm;
    tex_desc.mipLevelCount = 1;
    tex_desc.usage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding;
    bd->renderResources.FontTexture =
        wgpuDeviceCreateTexture(bd->wgpuDevice, &tex_desc);

    WGPUTextureViewDescriptor tex_view_desc = {};
    tex_view_desc.format = WGPUTextureFormat_RGBA8Unorm;
    tex_view_desc.dimension = WGPUTextureViewDimension_2D;
    tex_view_desc.baseMipLevel = 0;
    tex_view_desc.mipLevelCount = 1;
    tex_view_desc.baseArrayLayer = 0;
    tex_view_desc.arrayLayerCount = 1;
    tex_view_desc.aspect = WGPUTextureAspect_All;
    bd->renderResources.FontTextureView =
        wgpuTextureCreateView(bd->renderResources.FontTexture, &tex_view_desc);
  }

  // Upload texture data
  {
    WGPUImageCopyTexture dst_view = {};
    dst_view.texture = bd->renderResources.FontTexture;
    dst_view.mipLevel = 0;
    dst_view.origin = {0, 0, 0};
    dst_view.aspect = WGPUTextureAspect_All;
    WGPUTextureDataLayout layout = {};
    layout.offset = 0;
    layout.bytesPerRow = width * size_pp;
    layout.rowsPerImage = height;
    WGPUExtent3D size = {(uint32_t)width, (uint32_t)height, 1};
    wgpuQueueWriteTexture(bd->defaultQueue, &dst_view, pixels,
                          (uint32_t)(width * size_pp * height), &layout, &size);
  }

  // Create the associated sampler
  // (Bilinear sampling is required by default. Set 'io.Fonts->Flags |=
  // FontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to
  // allow point/nearest sampling)
  {
    WGPUSamplerDescriptor sampler_desc = {};
    sampler_desc.minFilter = WGPUFilterMode_Linear;
    sampler_desc.magFilter = WGPUFilterMode_Linear;
    sampler_desc.mipmapFilter = WGPUMipmapFilterMode_Linear;
    sampler_desc.addressModeU = WGPUAddressMode_Repeat;
    sampler_desc.addressModeV = WGPUAddressMode_Repeat;
    sampler_desc.addressModeW = WGPUAddressMode_Repeat;
    sampler_desc.maxAnisotropy = 1;
    bd->renderResources.Sampler =
        wgpuDeviceCreateSampler(bd->wgpuDevice, &sampler_desc);
  }

  // Store our identifier
  static_assert(
      sizeof(TextureID) >= sizeof(bd->renderResources.FontTexture),
      "Can't pack descriptor handle into TexID, 32-bit not supported yet.");
  io.Fonts->SetTexID((TextureID)bd->renderResources.FontTextureView);
}

static void WGPU_CreateUniformBuffer() {
  WGPU_Data *bd = WGPU_GetBackendData();
  WGPUBufferDescriptor ub_desc = {nullptr, "Gui Uniform buffer",
                                  WGPUBufferUsage_CopyDst |
                                      WGPUBufferUsage_Uniform,
                                  MEMALIGN(sizeof(Uniforms), 16), false};
  bd->renderResources.Uniforms =
      wgpuDeviceCreateBuffer(bd->wgpuDevice, &ub_desc);
}

bool WGPU_CreateDeviceObjects() {
  WGPU_Data *bd = WGPU_GetBackendData();
  if (!bd->wgpuDevice)
    return false;
  if (bd->pipelineState)
    WGPU_InvalidateDeviceObjects();

  // Create render pipeline
  WGPURenderPipelineDescriptor graphics_pipeline_desc = {};
  graphics_pipeline_desc.primitive.topology =
      WGPUPrimitiveTopology_TriangleList;
  graphics_pipeline_desc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
  graphics_pipeline_desc.primitive.frontFace = WGPUFrontFace_CW;
  graphics_pipeline_desc.primitive.cullMode = WGPUCullMode_None;
  graphics_pipeline_desc.multisample.count = 1;
  graphics_pipeline_desc.multisample.mask = UINT_MAX;
  graphics_pipeline_desc.multisample.alphaToCoverageEnabled = false;

  // Bind group layouts
  WGPUBindGroupLayoutEntry common_bg_layout_entries[2] = {};
  common_bg_layout_entries[0].binding = 0;
  common_bg_layout_entries[0].visibility =
      WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
  common_bg_layout_entries[0].buffer.type = WGPUBufferBindingType_Uniform;
  common_bg_layout_entries[1].binding = 1;
  common_bg_layout_entries[1].visibility = WGPUShaderStage_Fragment;
  common_bg_layout_entries[1].sampler.type = WGPUSamplerBindingType_Filtering;

  WGPUBindGroupLayoutEntry image_bg_layout_entries[1] = {};
  image_bg_layout_entries[0].binding = 0;
  image_bg_layout_entries[0].visibility = WGPUShaderStage_Fragment;
  image_bg_layout_entries[0].texture.sampleType = WGPUTextureSampleType_Float;
  image_bg_layout_entries[0].texture.viewDimension =
      WGPUTextureViewDimension_2D;

  WGPUBindGroupLayoutDescriptor common_bg_layout_desc = {};
  common_bg_layout_desc.entryCount = 2;
  common_bg_layout_desc.entries = common_bg_layout_entries;

  WGPUBindGroupLayoutDescriptor image_bg_layout_desc = {};
  image_bg_layout_desc.entryCount = 1;
  image_bg_layout_desc.entries = image_bg_layout_entries;

  WGPUBindGroupLayout bg_layouts[2];
  bg_layouts[0] =
      wgpuDeviceCreateBindGroupLayout(bd->wgpuDevice, &common_bg_layout_desc);
  bg_layouts[1] =
      wgpuDeviceCreateBindGroupLayout(bd->wgpuDevice, &image_bg_layout_desc);

  WGPUPipelineLayoutDescriptor layout_desc = {};
  layout_desc.bindGroupLayoutCount = 2;
  layout_desc.bindGroupLayouts = bg_layouts;
  graphics_pipeline_desc.layout =
      wgpuDeviceCreatePipelineLayout(bd->wgpuDevice, &layout_desc);

  // Create the vertex shader
  WGPUProgrammableStageDescriptor vertex_shader_desc =
      WGPU_CreateShaderModule(__shader_vert_wgsl);
  graphics_pipeline_desc.vertex.module = vertex_shader_desc.module;
  graphics_pipeline_desc.vertex.entryPoint = vertex_shader_desc.entryPoint;

  // Vertex input configuration
  WGPUVertexAttribute attribute_desc[] = {
      {WGPUVertexFormat_Float32x2, (uint64_t)offsetof(DrawVert, pos), 0},
      {WGPUVertexFormat_Float32x2, (uint64_t)offsetof(DrawVert, uv), 1},
      {WGPUVertexFormat_Unorm8x4, (uint64_t)offsetof(DrawVert, col), 2},
  };

  WGPUVertexBufferLayout buffer_layouts[1];
  buffer_layouts[0].arrayStride = sizeof(DrawVert);
  buffer_layouts[0].stepMode = WGPUVertexStepMode_Vertex;
  buffer_layouts[0].attributeCount = 3;
  buffer_layouts[0].attributes = attribute_desc;

  graphics_pipeline_desc.vertex.bufferCount = 1;
  graphics_pipeline_desc.vertex.buffers = buffer_layouts;

  // Create the pixel shader
  WGPUProgrammableStageDescriptor pixel_shader_desc =
      WGPU_CreateShaderModule(__shader_frag_wgsl);

  // Create the blending setup
  WGPUBlendState blend_state = {};
  blend_state.alpha.operation = WGPUBlendOperation_Add;
  blend_state.alpha.srcFactor = WGPUBlendFactor_One;
  blend_state.alpha.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
  blend_state.color.operation = WGPUBlendOperation_Add;
  blend_state.color.srcFactor = WGPUBlendFactor_SrcAlpha;
  blend_state.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;

  WGPUColorTargetState color_state = {};
  color_state.format = bd->renderTargetFormat;
  color_state.blend = &blend_state;
  color_state.writeMask = WGPUColorWriteMask_All;

  WGPUFragmentState fragment_state = {};
  fragment_state.module = pixel_shader_desc.module;
  fragment_state.entryPoint = pixel_shader_desc.entryPoint;
  fragment_state.targetCount = 1;
  fragment_state.targets = &color_state;

  graphics_pipeline_desc.fragment = &fragment_state;

  // Create depth-stencil State
  WGPUDepthStencilState depth_stencil_state = {};
  depth_stencil_state.format = bd->depthStencilFormat;
  depth_stencil_state.depthWriteEnabled = false;
  depth_stencil_state.depthCompare = WGPUCompareFunction_Always;
  depth_stencil_state.stencilFront.compare = WGPUCompareFunction_Always;
  depth_stencil_state.stencilBack.compare = WGPUCompareFunction_Always;

  // Configure disabled depth-stencil state
  graphics_pipeline_desc.depthStencil =
      (bd->depthStencilFormat == WGPUTextureFormat_Undefined)
          ? nullptr
          : &depth_stencil_state;

  bd->pipelineState =
      wgpuDeviceCreateRenderPipeline(bd->wgpuDevice, &graphics_pipeline_desc);

  WGPU_CreateFontsTexture();
  WGPU_CreateUniformBuffer();

  // Create resource bind group
  WGPUBindGroupEntry common_bg_entries[] = {
      {nullptr, 0, bd->renderResources.Uniforms, 0,
       MEMALIGN(sizeof(Uniforms), 16), 0, 0},
      {nullptr, 1, 0, 0, 0, bd->renderResources.Sampler, 0},
  };

  WGPUBindGroupDescriptor common_bg_descriptor = {};
  common_bg_descriptor.layout = bg_layouts[0];
  common_bg_descriptor.entryCount =
      sizeof(common_bg_entries) / sizeof(WGPUBindGroupEntry);
  common_bg_descriptor.entries = common_bg_entries;
  bd->renderResources.CommonBindGroup =
      wgpuDeviceCreateBindGroup(bd->wgpuDevice, &common_bg_descriptor);

  WGPUBindGroup image_bind_group = WGPU_CreateImageBindGroup(
      bg_layouts[1], bd->renderResources.FontTextureView);
  bd->renderResources.ImageBindGroup = image_bind_group;
  bd->renderResources.ImageBindGroupLayout = bg_layouts[1];
  bd->renderResources.ImageBindGroups.SetVoidPtr(
      HashData(&bd->renderResources.FontTextureView, sizeof(TextureID)),
      image_bind_group);

  SafeRelease(vertex_shader_desc.module);
  SafeRelease(pixel_shader_desc.module);
  SafeRelease(bg_layouts[0]);

  return true;
}

void WGPU_InvalidateDeviceObjects() {
  WGPU_Data *bd = WGPU_GetBackendData();
  if (!bd->wgpuDevice)
    return;

  SafeRelease(bd->pipelineState);
  SafeRelease(bd->renderResources);

  IO &io = Gui::GetIO();
  io.Fonts->SetTexID(0); // We copied g_pFontTextureView to io.Fonts->TexID so
                         // let's clear that as well.

  for (unsigned int i = 0; i < bd->numFramesInFlight; i++)
    SafeRelease(bd->pFrameResources[i]);
}

bool WGPU_Init(WGPUDevice device, int num_frames_in_flight,
               WGPUTextureFormat rt_format, WGPUTextureFormat depth_format) {
  IO &io = Gui::GetIO();
  assert(io.BackendRendererUserData == nullptr &&
         "Already initialized a renderer backend!");

  // Setup backend capabilities flags
  WGPU_Data *bd = NEW(WGPU_Data)();
  io.BackendRendererUserData = (void *)bd;
  io.BackendRendererName = "webgpu";
  io.BackendFlags |=
      BackendFlags_RendererHasVtxOffset; // We can honor the
                                         // DrawCmd::VtxOffset field,
                                         // allowing for large meshes.

  bd->wgpuDevice = device;
  bd->defaultQueue = wgpuDeviceGetQueue(bd->wgpuDevice);
  bd->renderTargetFormat = rt_format;
  bd->depthStencilFormat = depth_format;
  bd->numFramesInFlight = num_frames_in_flight;
  bd->frameIndex = UINT_MAX;

  bd->renderResources.FontTexture = nullptr;
  bd->renderResources.FontTextureView = nullptr;
  bd->renderResources.Sampler = nullptr;
  bd->renderResources.Uniforms = nullptr;
  bd->renderResources.CommonBindGroup = nullptr;
  bd->renderResources.ImageBindGroups.Data.reserve(100);
  bd->renderResources.ImageBindGroup = nullptr;
  bd->renderResources.ImageBindGroupLayout = nullptr;

  // Create buffers with a default size (they will later be grown as needed)
  bd->pFrameResources = new FrameResources[num_frames_in_flight];
  for (int i = 0; i < num_frames_in_flight; i++) {
    FrameResources *fr = &bd->pFrameResources[i];
    fr->IndexBuffer = nullptr;
    fr->VertexBuffer = nullptr;
    fr->IndexBufferHost = nullptr;
    fr->VertexBufferHost = nullptr;
    fr->IndexBufferSize = 10000;
    fr->VertexBufferSize = 5000;
  }

  return true;
}

void WGPU_Shutdown() {
  WGPU_Data *bd = WGPU_GetBackendData();
  assert(bd != nullptr &&
         "No renderer backend to shutdown, or already shutdown?");
  IO &io = Gui::GetIO();

  WGPU_InvalidateDeviceObjects();
  delete[] bd->pFrameResources;
  bd->pFrameResources = nullptr;
  wgpuQueueRelease(bd->defaultQueue);
  bd->wgpuDevice = nullptr;
  bd->numFramesInFlight = 0;
  bd->frameIndex = UINT_MAX;

  io.BackendRendererName = nullptr;
  io.BackendRendererUserData = nullptr;
  io.BackendFlags &= ~BackendFlags_RendererHasVtxOffset;
  DELETE(bd);
}

void WGPU_NewFrame() {
  WGPU_Data *bd = WGPU_GetBackendData();
  if (!bd->pipelineState)
    WGPU_CreateDeviceObjects();
}

//-----------------------------------------------------------------------------

#endif // #ifndef DISABLE
