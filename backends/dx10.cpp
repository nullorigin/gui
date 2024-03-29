// gui: Renderer Backend for DirectX10
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'ID3D10ShaderResourceView*' as
//  TextureID. Read the FAQ about TextureID! [X] Renderer: Large meshes
//  support (64k+ vertices) with 16-bit indices. [X] Renderer: Multi-viewport
//  support (multiple windows). Enable with 'io.ConfigFlags |=
//  ConfigFlags_ViewportsEnable'.

#include "../gui.hpp"
#ifndef DISABLE
#include "dx10.hpp"

// DirectX
#include <d3d10.h>
#include <d3d10_1.h>
#include <d3dcompiler.h>
#include <stdio.h>
#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") // Automatically link with d3dcompiler.lib
                                    // as we are using D3DCompile() below.
#endif

// DirectX data
struct DX10_Data {
  ID3D10Device *pd3dDevice;
  IDXGIFactory *pFactory;
  ID3D10Buffer *pVB;
  ID3D10Buffer *pIB;
  ID3D10VertexShader *pVertexShader;
  ID3D10InputLayout *pInputLayout;
  ID3D10Buffer *pVertexConstantBuffer;
  ID3D10PixelShader *pPixelShader;
  ID3D10SamplerState *pFontSampler;
  ID3D10ShaderResourceView *pFontTextureView;
  ID3D10RasterizerState *pRasterizerState;
  ID3D10BlendState *pBlendState;
  ID3D10DepthStencilState *pDepthStencilState;
  int VertexBufferSize;
  int IndexBufferSize;

  DX10_Data() {
    memset((void *)this, 0, sizeof(*this));
    VertexBufferSize = 5000;
    IndexBufferSize = 10000;
  }
};

struct VERTEX_CONSTANT_BUFFER_DX10 {
  float mvp[4][4];
};

// Backend data stored in io.BackendRendererUserData to allow support for
// multiple Gui contexts It is STRONGLY preferred that you use docking
// branch with multi-viewports (== single Gui context + multiple windows)
// instead of multiple Gui contexts.
static DX10_Data *DX10_GetBackendData() {
  return Gui::GetCurrentContext()
             ? (DX10_Data *)Gui::GetIO().BackendRendererUserData
             : nullptr;
}

// Forward Declarations
static void DX10_InitPlatformInterface();
static void DX10_ShutdownPlatformInterface();

// Functions
static void DX10_SetupRenderState(DrawData *draw_data, ID3D10Device *ctx) {
  DX10_Data *bd = DX10_GetBackendData();

  // Setup viewport
  D3D10_VIEWPORT vp;
  memset(&vp, 0, sizeof(D3D10_VIEWPORT));
  vp.Width = (UINT)draw_data->DisplaySize.x;
  vp.Height = (UINT)draw_data->DisplaySize.y;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = vp.TopLeftY = 0;
  ctx->RSSetViewports(1, &vp);

  // Bind shader and vertex buffers
  unsigned int stride = sizeof(DrawVert);
  unsigned int offset = 0;
  ctx->IASetInputLayout(bd->pInputLayout);
  ctx->IASetVertexBuffers(0, 1, &bd->pVB, &stride, &offset);
  ctx->IASetIndexBuffer(
      bd->pIB,
      sizeof(DrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
  ctx->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  ctx->VSSetShader(bd->pVertexShader);
  ctx->VSSetConstantBuffers(0, 1, &bd->pVertexConstantBuffer);
  ctx->PSSetShader(bd->pPixelShader);
  ctx->PSSetSamplers(0, 1, &bd->pFontSampler);
  ctx->GSSetShader(nullptr);

  // Setup render state
  const float blend_factor[4] = {0.f, 0.f, 0.f, 0.f};
  ctx->OMSetBlendState(bd->pBlendState, blend_factor, 0xffffffff);
  ctx->OMSetDepthStencilState(bd->pDepthStencilState, 0);
  ctx->RSSetState(bd->pRasterizerState);
}

// Render function
void DX10_RenderDrawData(DrawData *draw_data) {
  // Avoid rendering when minimized
  if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
    return;

  DX10_Data *bd = DX10_GetBackendData();
  ID3D10Device *ctx = bd->pd3dDevice;

  // Create and grow vertex/index buffers if needed
  if (!bd->pVB || bd->VertexBufferSize < draw_data->TotalVtxCount) {
    if (bd->pVB) {
      bd->pVB->Release();
      bd->pVB = nullptr;
    }
    bd->VertexBufferSize = draw_data->TotalVtxCount + 5000;
    D3D10_BUFFER_DESC desc;
    memset(&desc, 0, sizeof(D3D10_BUFFER_DESC));
    desc.Usage = D3D10_USAGE_DYNAMIC;
    desc.ByteWidth = bd->VertexBufferSize * sizeof(DrawVert);
    desc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    if (ctx->CreateBuffer(&desc, nullptr, &bd->pVB) < 0)
      return;
  }

  if (!bd->pIB || bd->IndexBufferSize < draw_data->TotalIdxCount) {
    if (bd->pIB) {
      bd->pIB->Release();
      bd->pIB = nullptr;
    }
    bd->IndexBufferSize = draw_data->TotalIdxCount + 10000;
    D3D10_BUFFER_DESC desc;
    memset(&desc, 0, sizeof(D3D10_BUFFER_DESC));
    desc.Usage = D3D10_USAGE_DYNAMIC;
    desc.ByteWidth = bd->IndexBufferSize * sizeof(DrawIdx);
    desc.BindFlags = D3D10_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    if (ctx->CreateBuffer(&desc, nullptr, &bd->pIB) < 0)
      return;
  }

  // Copy and convert all vertices into a single contiguous buffer
  DrawVert *vtx_dst = nullptr;
  DrawIdx *idx_dst = nullptr;
  bd->pVB->Map(D3D10_MAP_WRITE_DISCARD, 0, (void **)&vtx_dst);
  bd->pIB->Map(D3D10_MAP_WRITE_DISCARD, 0, (void **)&idx_dst);
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const DrawList *cmd_list = draw_data->CmdLists[n];
    memcpy(vtx_dst, cmd_list->VtxBuffer.Data,
           cmd_list->VtxBuffer.Size * sizeof(DrawVert));
    memcpy(idx_dst, cmd_list->IdxBuffer.Data,
           cmd_list->IdxBuffer.Size * sizeof(DrawIdx));
    vtx_dst += cmd_list->VtxBuffer.Size;
    idx_dst += cmd_list->IdxBuffer.Size;
  }
  bd->pVB->Unmap();
  bd->pIB->Unmap();

  // Setup orthographic projection matrix into our constant buffer
  // Our visible imgui space lies from draw_data->DisplayPos (top left) to
  // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is
  // (0,0) for single viewport apps.
  {
    void *mapped_resource;
    if (bd->pVertexConstantBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0,
                                       &mapped_resource) != S_OK)
      return;
    VERTEX_CONSTANT_BUFFER_DX10 *constant_buffer =
        (VERTEX_CONSTANT_BUFFER_DX10 *)mapped_resource;
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
    memcpy(&constant_buffer->mvp, mvp, sizeof(mvp));
    bd->pVertexConstantBuffer->Unmap();
  }

  // Backup DX state that will be modified to restore it afterwards
  // (unfortunately this is very ugly looking and verbose. Close your eyes!)
  struct BACKUP_DX10_STATE {
    UINT ScissorRectsCount, ViewportsCount;
    D3D10_RECT
    ScissorRects[D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    D3D10_VIEWPORT
    Viewports[D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    ID3D10RasterizerState *RS;
    ID3D10BlendState *BlendState;
    FLOAT BlendFactor[4];
    UINT SampleMask;
    UINT StencilRef;
    ID3D10DepthStencilState *DepthStencilState;
    ID3D10ShaderResourceView *PSShaderResource;
    ID3D10SamplerState *PSSampler;
    ID3D10PixelShader *PS;
    ID3D10VertexShader *VS;
    ID3D10GeometryShader *GS;
    D3D10_PRIMITIVE_TOPOLOGY PrimitiveTopology;
    ID3D10Buffer *IndexBuffer, *VertexBuffer, *VSConstantBuffer;
    UINT IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
    DXGI_FORMAT IndexBufferFormat;
    ID3D10InputLayout *InputLayout;
  };
  BACKUP_DX10_STATE old = {};
  old.ScissorRectsCount = old.ViewportsCount =
      D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
  ctx->RSGetScissorRects(&old.ScissorRectsCount, old.ScissorRects);
  ctx->RSGetViewports(&old.ViewportsCount, old.Viewports);
  ctx->RSGetState(&old.RS);
  ctx->OMGetBlendState(&old.BlendState, old.BlendFactor, &old.SampleMask);
  ctx->OMGetDepthStencilState(&old.DepthStencilState, &old.StencilRef);
  ctx->PSGetShaderResources(0, 1, &old.PSShaderResource);
  ctx->PSGetSamplers(0, 1, &old.PSSampler);
  ctx->PSGetShader(&old.PS);
  ctx->VSGetShader(&old.VS);
  ctx->VSGetConstantBuffers(0, 1, &old.VSConstantBuffer);
  ctx->GSGetShader(&old.GS);
  ctx->IAGetPrimitiveTopology(&old.PrimitiveTopology);
  ctx->IAGetIndexBuffer(&old.IndexBuffer, &old.IndexBufferFormat,
                        &old.IndexBufferOffset);
  ctx->IAGetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride,
                          &old.VertexBufferOffset);
  ctx->IAGetInputLayout(&old.InputLayout);

  // Setup desired DX state
  DX10_SetupRenderState(draw_data, ctx);

  // Render command lists
  // (Because we merged all buffers into a single one, we maintain our own
  // offset into them)
  int global_vtx_offset = 0;
  int global_idx_offset = 0;
  Vec2 clip_off = draw_data->DisplayPos;
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const DrawList *cmd_list = draw_data->CmdLists[n];
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const DrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback) {
        // User callback, registered via DrawList::AddCallback()
        // (DrawCallback_ResetRenderState is a special callback value used by
        // the user to request the renderer to reset render state.)
        if (pcmd->UserCallback == DrawCallback_ResetRenderState)
          DX10_SetupRenderState(draw_data, ctx);
        else
          pcmd->UserCallback(cmd_list, pcmd);
      } else {
        // Project scissor/clipping rectangles into framebuffer space
        Vec2 clip_min(pcmd->ClipRect.x - clip_off.x,
                      pcmd->ClipRect.y - clip_off.y);
        Vec2 clip_max(pcmd->ClipRect.z - clip_off.x,
                      pcmd->ClipRect.w - clip_off.y);
        if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
          continue;

        // Apply scissor/clipping rectangle
        const D3D10_RECT r = {(LONG)clip_min.x, (LONG)clip_min.y,
                              (LONG)clip_max.x, (LONG)clip_max.y};
        ctx->RSSetScissorRects(1, &r);

        // Bind texture, Draw
        ID3D10ShaderResourceView *texture_srv =
            (ID3D10ShaderResourceView *)pcmd->GetTexID();
        ctx->PSSetShaderResources(0, 1, &texture_srv);
        ctx->DrawIndexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset,
                         pcmd->VtxOffset + global_vtx_offset);
      }
    }
    global_idx_offset += cmd_list->IdxBuffer.Size;
    global_vtx_offset += cmd_list->VtxBuffer.Size;
  }

  // Restore modified DX state
  ctx->RSSetScissorRects(old.ScissorRectsCount, old.ScissorRects);
  ctx->RSSetViewports(old.ViewportsCount, old.Viewports);
  ctx->RSSetState(old.RS);
  if (old.RS)
    old.RS->Release();
  ctx->OMSetBlendState(old.BlendState, old.BlendFactor, old.SampleMask);
  if (old.BlendState)
    old.BlendState->Release();
  ctx->OMSetDepthStencilState(old.DepthStencilState, old.StencilRef);
  if (old.DepthStencilState)
    old.DepthStencilState->Release();
  ctx->PSSetShaderResources(0, 1, &old.PSShaderResource);
  if (old.PSShaderResource)
    old.PSShaderResource->Release();
  ctx->PSSetSamplers(0, 1, &old.PSSampler);
  if (old.PSSampler)
    old.PSSampler->Release();
  ctx->PSSetShader(old.PS);
  if (old.PS)
    old.PS->Release();
  ctx->VSSetShader(old.VS);
  if (old.VS)
    old.VS->Release();
  ctx->GSSetShader(old.GS);
  if (old.GS)
    old.GS->Release();
  ctx->VSSetConstantBuffers(0, 1, &old.VSConstantBuffer);
  if (old.VSConstantBuffer)
    old.VSConstantBuffer->Release();
  ctx->IASetPrimitiveTopology(old.PrimitiveTopology);
  ctx->IASetIndexBuffer(old.IndexBuffer, old.IndexBufferFormat,
                        old.IndexBufferOffset);
  if (old.IndexBuffer)
    old.IndexBuffer->Release();
  ctx->IASetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride,
                          &old.VertexBufferOffset);
  if (old.VertexBuffer)
    old.VertexBuffer->Release();
  ctx->IASetInputLayout(old.InputLayout);
  if (old.InputLayout)
    old.InputLayout->Release();
}

static void DX10_CreateFontsTexture() {
  // Build texture atlas
  DX10_Data *bd = DX10_GetBackendData();
  IO &io = Gui::GetIO();
  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  // Upload texture to graphics system
  {
    D3D10_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D10_USAGE_DEFAULT;
    desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D10Texture2D *pTexture = nullptr;
    D3D10_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = pixels;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    bd->pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);
    assert(pTexture != nullptr);

    // Create texture view
    D3D10_SHADER_RESOURCE_VIEW_DESC srv_desc;
    ZeroMemory(&srv_desc, sizeof(srv_desc));
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = desc.MipLevels;
    srv_desc.Texture2D.MostDetailedMip = 0;
    bd->pd3dDevice->CreateShaderResourceView(pTexture, &srv_desc,
                                             &bd->pFontTextureView);
    pTexture->Release();
  }

  // Store our identifier
  io.Fonts->SetTexID((TextureID)bd->pFontTextureView);

  // Create texture sampler
  // (Bilinear sampling is required by default. Set 'io.Fonts->Flags |=
  // FontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to
  // allow point/nearest sampling)
  {
    D3D10_SAMPLER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Filter = D3D10_FILTER_MIN_MAG_MIP_LINEAR;
    desc.AddressU = D3D10_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D10_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D10_TEXTURE_ADDRESS_WRAP;
    desc.MipLODBias = 0.f;
    desc.ComparisonFunc = D3D10_COMPARISON_ALWAYS;
    desc.MinLOD = 0.f;
    desc.MaxLOD = 0.f;
    bd->pd3dDevice->CreateSamplerState(&desc, &bd->pFontSampler);
  }
}

bool DX10_CreateDeviceObjects() {
  DX10_Data *bd = DX10_GetBackendData();
  if (!bd->pd3dDevice)
    return false;
  if (bd->pFontSampler)
    DX10_InvalidateDeviceObjects();

  // By using D3DCompile() from <d3dcompiler.h> / d3dcompiler.lib, we introduce
  // a dependency to a given version of d3dcompiler_XX.dll (see
  // D3DCOMPILER_DLL_A) If you would like to use this DX10 sample code but
  // remove this dependency you can:
  //  1) compile once, save the compiled shader blobs into a file or source code
  //  and pass them to CreateVertexShader()/CreatePixelShader() [preferred
  //  solution] 2) use code to detect any version of the DLL and grab a pointer
  //  to D3DCompile from the DLL.
  //
  // Create the vertex shader
  {
    static const char *vertexShader = "cbuffer vertexBuffer : register(b0) \
            {\
              float4x4 ProjectionMatrix; \
            };\
            struct VS_INPUT\
            {\
              float2 pos : POSITION;\
              float4 col : COLOR0;\
              float2 uv  : TEXCOORD0;\
            };\
            \
            struct PS_INPUT\
            {\
              float4 pos : SV_POSITION;\
              float4 col : COLOR0;\
              float2 uv  : TEXCOORD0;\
            };\
            \
            PS_INPUT main(VS_INPUT input)\
            {\
              PS_INPUT output;\
              output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
              output.col = input.col;\
              output.uv  = input.uv;\
              return output;\
            }";

    ID3DBlob *vertexShaderBlob;
    if (FAILED(D3DCompile(vertexShader, strlen(vertexShader), nullptr, nullptr,
                          nullptr, "main", "vs_4_0", 0, 0, &vertexShaderBlob,
                          nullptr)))
      return false; // NB: Pass ID3DBlob* pErrorBlob to D3DCompile() to get
                    // error showing in (const
                    // char*)pErrorBlob->GetBufferPointer(). Make sure to
                    // Release() the blob!
    if (bd->pd3dDevice->CreateVertexShader(vertexShaderBlob->GetBufferPointer(),
                                           vertexShaderBlob->GetBufferSize(),
                                           &bd->pVertexShader) != S_OK) {
      vertexShaderBlob->Release();
      return false;
    }

    // Create the input layout
    D3D10_INPUT_ELEMENT_DESC local_layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
         (UINT)offsetof(DrawVert, pos), D3D10_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
         (UINT)offsetof(DrawVert, uv), D3D10_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0,
         (UINT)offsetof(DrawVert, col), D3D10_INPUT_PER_VERTEX_DATA, 0},
    };
    if (bd->pd3dDevice->CreateInputLayout(
            local_layout, 3, vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(), &bd->pInputLayout) != S_OK) {
      vertexShaderBlob->Release();
      return false;
    }
    vertexShaderBlob->Release();

    // Create the constant buffer
    {
      D3D10_BUFFER_DESC desc;
      desc.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER_DX10);
      desc.Usage = D3D10_USAGE_DYNAMIC;
      desc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
      desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
      desc.MiscFlags = 0;
      bd->pd3dDevice->CreateBuffer(&desc, nullptr, &bd->pVertexConstantBuffer);
    }
  }

  // Create the pixel shader
  {
    static const char *pixelShader = "struct PS_INPUT\
            {\
            float4 pos : SV_POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            sampler sampler0;\
            Texture2D texture0;\
            \
            float4 main(PS_INPUT input) : SV_Target\
            {\
            float4 out_col = input.col * texture0.Sample(sampler0, input.uv); \
            return out_col; \
            }";

    ID3DBlob *pixelShaderBlob;
    if (FAILED(D3DCompile(pixelShader, strlen(pixelShader), nullptr, nullptr,
                          nullptr, "main", "ps_4_0", 0, 0, &pixelShaderBlob,
                          nullptr)))
      return false; // NB: Pass ID3DBlob* pErrorBlob to D3DCompile() to get
                    // error showing in (const
                    // char*)pErrorBlob->GetBufferPointer(). Make sure to
                    // Release() the blob!
    if (bd->pd3dDevice->CreatePixelShader(pixelShaderBlob->GetBufferPointer(),
                                          pixelShaderBlob->GetBufferSize(),
                                          &bd->pPixelShader) != S_OK) {
      pixelShaderBlob->Release();
      return false;
    }
    pixelShaderBlob->Release();
  }

  // Create the blending setup
  {
    D3D10_BLEND_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.AlphaToCoverageEnable = false;
    desc.BlendEnable[0] = true;
    desc.SrcBlend = D3D10_BLEND_SRC_ALPHA;
    desc.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
    desc.BlendOp = D3D10_BLEND_OP_ADD;
    desc.SrcBlendAlpha = D3D10_BLEND_ONE;
    desc.DestBlendAlpha = D3D10_BLEND_INV_SRC_ALPHA;
    desc.BlendOpAlpha = D3D10_BLEND_OP_ADD;
    desc.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;
    bd->pd3dDevice->CreateBlendState(&desc, &bd->pBlendState);
  }

  // Create the rasterizer state
  {
    D3D10_RASTERIZER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.FillMode = D3D10_FILL_SOLID;
    desc.CullMode = D3D10_CULL_NONE;
    desc.ScissorEnable = true;
    desc.DepthClipEnable = true;
    bd->pd3dDevice->CreateRasterizerState(&desc, &bd->pRasterizerState);
  }

  // Create depth-stencil State
  {
    D3D10_DEPTH_STENCIL_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.DepthEnable = false;
    desc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D10_COMPARISON_ALWAYS;
    desc.StencilEnable = false;
    desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp =
        desc.FrontFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFunc = D3D10_COMPARISON_ALWAYS;
    desc.BackFace = desc.FrontFace;
    bd->pd3dDevice->CreateDepthStencilState(&desc, &bd->pDepthStencilState);
  }

  DX10_CreateFontsTexture();

  return true;
}

void DX10_InvalidateDeviceObjects() {
  DX10_Data *bd = DX10_GetBackendData();
  if (!bd->pd3dDevice)
    return;

  if (bd->pFontSampler) {
    bd->pFontSampler->Release();
    bd->pFontSampler = nullptr;
  }
  if (bd->pFontTextureView) {
    bd->pFontTextureView->Release();
    bd->pFontTextureView = nullptr;
    Gui::GetIO().Fonts->SetTexID(0);
  } // We copied bd->pFontTextureView to io.Fonts->TexID so let's clear that as
    // well.
  if (bd->pIB) {
    bd->pIB->Release();
    bd->pIB = nullptr;
  }
  if (bd->pVB) {
    bd->pVB->Release();
    bd->pVB = nullptr;
  }
  if (bd->pBlendState) {
    bd->pBlendState->Release();
    bd->pBlendState = nullptr;
  }
  if (bd->pDepthStencilState) {
    bd->pDepthStencilState->Release();
    bd->pDepthStencilState = nullptr;
  }
  if (bd->pRasterizerState) {
    bd->pRasterizerState->Release();
    bd->pRasterizerState = nullptr;
  }
  if (bd->pPixelShader) {
    bd->pPixelShader->Release();
    bd->pPixelShader = nullptr;
  }
  if (bd->pVertexConstantBuffer) {
    bd->pVertexConstantBuffer->Release();
    bd->pVertexConstantBuffer = nullptr;
  }
  if (bd->pInputLayout) {
    bd->pInputLayout->Release();
    bd->pInputLayout = nullptr;
  }
  if (bd->pVertexShader) {
    bd->pVertexShader->Release();
    bd->pVertexShader = nullptr;
  }
}

bool DX10_Init(ID3D10Device *device) {
  IO &io = Gui::GetIO();
  assert(io.BackendRendererUserData == nullptr &&
         "Already initialized a renderer backend!");

  // Setup backend capabilities flags
  DX10_Data *bd = NEW(DX10_Data)();
  io.BackendRendererUserData = (void *)bd;
  io.BackendRendererName = "dx10";
  io.BackendFlags |=
      BackendFlags_RendererHasVtxOffset; // We can honor the
                                         // DrawCmd::VtxOffset field,
                                         // allowing for large meshes.
  io.BackendFlags |=
      BackendFlags_RendererHasViewports; // We can create multi-viewports
                                         // on the Renderer side (optional)

  // Get factory from device
  IDXGIDevice *pDXGIDevice = nullptr;
  IDXGIAdapter *pDXGIAdapter = nullptr;
  IDXGIFactory *pFactory = nullptr;
  if (device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice)) == S_OK)
    if (pDXGIDevice->GetParent(IID_PPV_ARGS(&pDXGIAdapter)) == S_OK)
      if (pDXGIAdapter->GetParent(IID_PPV_ARGS(&pFactory)) == S_OK) {
        bd->pd3dDevice = device;
        bd->pFactory = pFactory;
      }
  if (pDXGIDevice)
    pDXGIDevice->Release();
  if (pDXGIAdapter)
    pDXGIAdapter->Release();
  bd->pd3dDevice->AddRef();

  if (io.ConfigFlags & ConfigFlags_ViewportsEnable)
    DX10_InitPlatformInterface();
  return true;
}

void DX10_Shutdown() {
  DX10_Data *bd = DX10_GetBackendData();
  assert(bd != nullptr &&
         "No renderer backend to shutdown, or already shutdown?");
  IO &io = Gui::GetIO();

  DX10_ShutdownPlatformInterface();
  DX10_InvalidateDeviceObjects();
  if (bd->pFactory) {
    bd->pFactory->Release();
  }
  if (bd->pd3dDevice) {
    bd->pd3dDevice->Release();
  }
  io.BackendRendererName = nullptr;
  io.BackendRendererUserData = nullptr;
  io.BackendFlags &=
      ~(BackendFlags_RendererHasVtxOffset | BackendFlags_RendererHasViewports);
  DELETE(bd);
}

void DX10_NewFrame() {
  DX10_Data *bd = DX10_GetBackendData();
  assert(bd != nullptr && "Did you call DX10_Init()?");

  if (!bd->pFontSampler)
    DX10_CreateDeviceObjects();
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create
// and handle multiple viewports simultaneously. If you are new to gui or
// creating a new binding for gui, it is recommended that you completely
// ignore this section first..
//--------------------------------------------------------------------------------------------------------

// Helper structure we store in the void* RendererUserData field of each
// Viewport to easily retrieve our backend data.
struct DX10_ViewportData {
  IDXGISwapChain *SwapChain;
  ID3D10RenderTargetView *RTView;

  DX10_ViewportData() {
    SwapChain = nullptr;
    RTView = nullptr;
  }
  ~DX10_ViewportData() { assert(SwapChain == nullptr && RTView == nullptr); }
};

static void DX10_CreateWindow(Viewport *viewport) {
  DX10_Data *bd = DX10_GetBackendData();
  DX10_ViewportData *vd = NEW(DX10_ViewportData)();
  viewport->RendererUserData = vd;

  // PlatformHandleRaw should always be a HWND, whereas PlatformHandle might be
  // a higher-level handle (e.g. GLFWWindow*, SDL_Window*). Some backends will
  // leave PlatformHandleRaw == 0, in which case we assume PlatformHandle will
  // contain the HWND.
  HWND hwnd = viewport->PlatformHandleRaw ? (HWND)viewport->PlatformHandleRaw
                                          : (HWND)viewport->PlatformHandle;
  assert(hwnd != 0);

  // Create swap chain
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferDesc.Width = (UINT)viewport->Size.x;
  sd.BufferDesc.Height = (UINT)viewport->Size.y;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.BufferCount = 1;
  sd.OutputWindow = hwnd;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  sd.Flags = 0;

  assert(vd->SwapChain == nullptr && vd->RTView == nullptr);
  bd->pFactory->CreateSwapChain(bd->pd3dDevice, &sd, &vd->SwapChain);

  // Create the render target
  if (vd->SwapChain) {
    ID3D10Texture2D *pBackBuffer;
    vd->SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    bd->pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &vd->RTView);
    pBackBuffer->Release();
  }
}

static void DX10_DestroyWindow(Viewport *viewport) {
  // The main viewport (owned by the application) will always have
  // RendererUserData == 0 here since we didn't create the data for it.
  if (DX10_ViewportData *vd = (DX10_ViewportData *)viewport->RendererUserData) {
    if (vd->SwapChain)
      vd->SwapChain->Release();
    vd->SwapChain = nullptr;
    if (vd->RTView)
      vd->RTView->Release();
    vd->RTView = nullptr;
    DELETE(vd);
  }
  viewport->RendererUserData = nullptr;
}

static void DX10_SetWindowSize(Viewport *viewport, Vec2 size) {
  DX10_Data *bd = DX10_GetBackendData();
  DX10_ViewportData *vd = (DX10_ViewportData *)viewport->RendererUserData;
  if (vd->RTView) {
    vd->RTView->Release();
    vd->RTView = nullptr;
  }
  if (vd->SwapChain) {
    ID3D10Texture2D *pBackBuffer = nullptr;
    vd->SwapChain->ResizeBuffers(0, (UINT)size.x, (UINT)size.y,
                                 DXGI_FORMAT_UNKNOWN, 0);
    vd->SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer == nullptr) {
      fprintf(stderr, "DX10_SetWindowSize() failed creating buffers.\n");
      return;
    }
    bd->pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &vd->RTView);
    pBackBuffer->Release();
  }
}

static void DX10_RenderViewport(Viewport *viewport, void *) {
  DX10_Data *bd = DX10_GetBackendData();
  DX10_ViewportData *vd = (DX10_ViewportData *)viewport->RendererUserData;
  Vec4 clear_color = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
  bd->pd3dDevice->OMSetRenderTargets(1, &vd->RTView, nullptr);
  if (!(viewport->Flags & ViewportFlags_NoRendererClear))
    bd->pd3dDevice->ClearRenderTargetView(vd->RTView, (float *)&clear_color);
  DX10_RenderDrawData(viewport->DrawData);
}

static void DX10_SwapBuffers(Viewport *viewport, void *) {
  DX10_ViewportData *vd = (DX10_ViewportData *)viewport->RendererUserData;
  vd->SwapChain->Present(0, 0); // Present without vsync
}

void DX10_InitPlatformInterface() {
  PlatformIO &platform_io = Gui::GetPlatformIO();
  platform_io.Renderer_CreateWindow = DX10_CreateWindow;
  platform_io.Renderer_DestroyWindow = DX10_DestroyWindow;
  platform_io.Renderer_SetWindowSize = DX10_SetWindowSize;
  platform_io.Renderer_RenderWindow = DX10_RenderViewport;
  platform_io.Renderer_SwapBuffers = DX10_SwapBuffers;
}

void DX10_ShutdownPlatformInterface() { Gui::DestroyPlatformWindows(); }

//-----------------------------------------------------------------------------

#endif // #ifndef DISABLE
