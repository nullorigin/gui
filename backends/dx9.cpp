// gui: Renderer Backend for DirectX9
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'LPDIRECT3DTEXTURE9' as TextureID.
//  Read the FAQ about TextureID! [X] Renderer: Large meshes support (64k+
//  vertices) with 16-bit indices. [X] Renderer: Multi-viewport support
//  (multiple windows). Enable with 'io.ConfigFlags |=
//  ConfigFlags_ViewportsEnable'.

#include "../gui.hpp"
#ifndef DISABLE
#include "dx9.hpp"

// DirectX
#include <d3d9.h>

// DirectX data
struct DX9_Data {
  LPDIRECT3DDEVICE9 pd3dDevice;
  LPDIRECT3DVERTEXBUFFER9 pVB;
  LPDIRECT3DINDEXBUFFER9 pIB;
  LPDIRECT3DTEXTURE9 FontTexture;
  int VertexBufferSize;
  int IndexBufferSize;

  DX9_Data() {
    memset((void *)this, 0, sizeof(*this));
    VertexBufferSize = 5000;
    IndexBufferSize = 10000;
  }
};

struct CUSTOMVERTEX {
  float pos[3];
  D3DCOLOR col;
  float uv[2];
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

#ifdef USE_BGRA_PACKED_COLOR
#define COL_TO_DX9_ARGB(_COL) (_COL)
#else
#define COL_TO_DX9_ARGB(_COL)                                                  \
  (((_COL) & 0xFF00FF00) | (((_COL) & 0xFF0000) >> 16) |                       \
   (((_COL) & 0xFF) << 16))
#endif

// Backend data stored in io.BackendRendererUserData to allow support for
// multiple Gui contexts It is STRONGLY preferred that you use docking
// branch with multi-viewports (== single Gui context + multiple windows)
// instead of multiple Gui contexts.
static DX9_Data *DX9_GetBackendData() {
  return Gui::GetCurrentContext()
             ? (DX9_Data *)Gui::GetIO().BackendRendererUserData
             : nullptr;
}

// Forward Declarations
static void DX9_InitPlatformInterface();
static void DX9_ShutdownPlatformInterface();
static void DX9_CreateDeviceObjectsForPlatformWindows();
static void DX9_InvalidateDeviceObjectsForPlatformWindows();

// Functions
static void DX9_SetupRenderState(DrawData *draw_data) {
  DX9_Data *bd = DX9_GetBackendData();

  // Setup viewport
  D3DVIEWPORT9 vp;
  vp.X = vp.Y = 0;
  vp.Width = (DWORD)draw_data->DisplaySize.x;
  vp.Height = (DWORD)draw_data->DisplaySize.y;
  vp.MinZ = 0.0f;
  vp.MaxZ = 1.0f;
  bd->pd3dDevice->SetViewport(&vp);

  // Setup render state: fixed-pipeline, alpha-blending, no face culling, no
  // depth testing, shade mode (for gradient), bilinear sampling.
  bd->pd3dDevice->SetPixelShader(nullptr);
  bd->pd3dDevice->SetVertexShader(nullptr);
  bd->pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
  bd->pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
  bd->pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
  bd->pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
  bd->pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
  bd->pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
  bd->pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
  bd->pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
  bd->pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
  bd->pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
  bd->pd3dDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
  bd->pd3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
  bd->pd3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
  bd->pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
  bd->pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
  bd->pd3dDevice->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
  bd->pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
  bd->pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
  bd->pd3dDevice->SetRenderState(D3DRS_CLIPPING, TRUE);
  bd->pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
  bd->pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  bd->pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  bd->pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  bd->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
  bd->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  bd->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
  bd->pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
  bd->pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  bd->pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  bd->pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

  // Setup orthographic projection matrix
  // Our visible imgui space lies from draw_data->DisplayPos (top left) to
  // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is
  // (0,0) for single viewport apps. Being agnostic of whether <d3dx9.h> or
  // <DirectXMath.h> can be used, we aren't relying on
  // D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or
  // DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
  {
    float L = draw_data->DisplayPos.x + 0.5f;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x + 0.5f;
    float T = draw_data->DisplayPos.y + 0.5f;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y + 0.5f;
    D3DMATRIX mat_identity = {
        {{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 1.0f}}};
    D3DMATRIX mat_projection = {
        {{2.0f / (R - L), 0.0f, 0.0f, 0.0f, 0.0f, 2.0f / (T - B), 0.0f, 0.0f,
          0.0f, 0.0f, 0.5f, 0.0f, (L + R) / (L - R), (T + B) / (B - T), 0.5f,
          1.0f}}};
    bd->pd3dDevice->SetTransform(D3DTS_WORLD, &mat_identity);
    bd->pd3dDevice->SetTransform(D3DTS_VIEW, &mat_identity);
    bd->pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat_projection);
  }
}

// Render function.
void DX9_RenderDrawData(DrawData *draw_data) {
  // Avoid rendering when minimized
  if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
    return;

  // Create and grow buffers if needed
  DX9_Data *bd = DX9_GetBackendData();
  if (!bd->pVB || bd->VertexBufferSize < draw_data->TotalVtxCount) {
    if (bd->pVB) {
      bd->pVB->Release();
      bd->pVB = nullptr;
    }
    bd->VertexBufferSize = draw_data->TotalVtxCount + 5000;
    if (bd->pd3dDevice->CreateVertexBuffer(
            bd->VertexBufferSize * sizeof(CUSTOMVERTEX),
            D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX,
            D3DPOOL_DEFAULT, &bd->pVB, nullptr) < 0)
      return;
  }
  if (!bd->pIB || bd->IndexBufferSize < draw_data->TotalIdxCount) {
    if (bd->pIB) {
      bd->pIB->Release();
      bd->pIB = nullptr;
    }
    bd->IndexBufferSize = draw_data->TotalIdxCount + 10000;
    if (bd->pd3dDevice->CreateIndexBuffer(
            bd->IndexBufferSize * sizeof(DrawIdx),
            D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
            sizeof(DrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32,
            D3DPOOL_DEFAULT, &bd->pIB, nullptr) < 0)
      return;
  }

  // Backup the DX9 state
  IDirect3DStateBlock9 *d3d9_state_block = nullptr;
  if (bd->pd3dDevice->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
    return;
  if (d3d9_state_block->Capture() < 0) {
    d3d9_state_block->Release();
    return;
  }

  // Backup the DX9 transform (DX9 documentation suggests that it is included in
  // the StateBlock but it doesn't appear to)
  D3DMATRIX last_world, last_view, last_projection;
  bd->pd3dDevice->GetTransform(D3DTS_WORLD, &last_world);
  bd->pd3dDevice->GetTransform(D3DTS_VIEW, &last_view);
  bd->pd3dDevice->GetTransform(D3DTS_PROJECTION, &last_projection);

  // Allocate buffers
  CUSTOMVERTEX *vtx_dst;
  DrawIdx *idx_dst;
  if (bd->pVB->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(CUSTOMVERTEX)),
                    (void **)&vtx_dst, D3DLOCK_DISCARD) < 0) {
    d3d9_state_block->Release();
    return;
  }
  if (bd->pIB->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(DrawIdx)),
                    (void **)&idx_dst, D3DLOCK_DISCARD) < 0) {
    bd->pVB->Unlock();
    d3d9_state_block->Release();
    return;
  }

  // Copy and convert all vertices into a single contiguous buffer, convert
  // colors to DX9 default format.
  // FIXME-OPT: This is a minor waste of resource, the ideal is to use
  // config.hpp and
  //  1) to avoid repacking colors:   #define USE_BGRA_PACKED_COLOR
  //  2) to avoid repacking vertices: #define OVERRIDE_DRAWVERT_STRUCT_LAYOUT
  //  struct DrawVert { Vec2 pos; float z; unsigned int col; Vec2 uv; }
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const DrawList *cmd_list = draw_data->CmdLists[n];
    const DrawVert *vtx_src = cmd_list->VtxBuffer.Data;
    for (int i = 0; i < cmd_list->VtxBuffer.Size; i++) {
      vtx_dst->pos[0] = vtx_src->pos.x;
      vtx_dst->pos[1] = vtx_src->pos.y;
      vtx_dst->pos[2] = 0.0f;
      vtx_dst->col = COL_TO_DX9_ARGB(vtx_src->col);
      vtx_dst->uv[0] = vtx_src->uv.x;
      vtx_dst->uv[1] = vtx_src->uv.y;
      vtx_dst++;
      vtx_src++;
    }
    memcpy(idx_dst, cmd_list->IdxBuffer.Data,
           cmd_list->IdxBuffer.Size * sizeof(DrawIdx));
    idx_dst += cmd_list->IdxBuffer.Size;
  }
  bd->pVB->Unlock();
  bd->pIB->Unlock();
  bd->pd3dDevice->SetStreamSource(0, bd->pVB, 0, sizeof(CUSTOMVERTEX));
  bd->pd3dDevice->SetIndices(bd->pIB);
  bd->pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

  // Setup desired DX state
  DX9_SetupRenderState(draw_data);

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
      if (pcmd->UserCallback != nullptr) {
        // User callback, registered via DrawList::AddCallback()
        // (DrawCallback_ResetRenderState is a special callback value used by
        // the user to request the renderer to reset render state.)
        if (pcmd->UserCallback == DrawCallback_ResetRenderState)
          DX9_SetupRenderState(draw_data);
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

        // Apply Scissor/clipping rectangle, Bind texture, Draw
        const RECT r = {(LONG)clip_min.x, (LONG)clip_min.y, (LONG)clip_max.x,
                        (LONG)clip_max.y};
        const LPDIRECT3DTEXTURE9 texture = (LPDIRECT3DTEXTURE9)pcmd->GetTexID();
        bd->pd3dDevice->SetTexture(0, texture);
        bd->pd3dDevice->SetScissorRect(&r);
        bd->pd3dDevice->DrawIndexedPrimitive(
            D3DPT_TRIANGLELIST, pcmd->VtxOffset + global_vtx_offset, 0,
            (UINT)cmd_list->VtxBuffer.Size, pcmd->IdxOffset + global_idx_offset,
            pcmd->ElemCount / 3);
      }
    }
    global_idx_offset += cmd_list->IdxBuffer.Size;
    global_vtx_offset += cmd_list->VtxBuffer.Size;
  }

  // When using multi-viewports, it appears that there's an odd logic in
  // DirectX9 which prevent subsequent windows from rendering until the first
  // window submits at least one draw call, even once. That's our workaround.
  // (see #2560)
  if (global_vtx_offset == 0)
    bd->pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 0, 0, 0);

  // Restore the DX9 transform
  bd->pd3dDevice->SetTransform(D3DTS_WORLD, &last_world);
  bd->pd3dDevice->SetTransform(D3DTS_VIEW, &last_view);
  bd->pd3dDevice->SetTransform(D3DTS_PROJECTION, &last_projection);

  // Restore the DX9 state
  d3d9_state_block->Apply();
  d3d9_state_block->Release();
}

bool DX9_Init(IDirect3DDevice9 *device) {
  IO &io = Gui::GetIO();
  ASSERT(io.BackendRendererUserData == nullptr &&
         "Already initialized a renderer backend!");

  // Setup backend capabilities flags
  DX9_Data *bd = NEW(DX9_Data)();
  io.BackendRendererUserData = (void *)bd;
  io.BackendRendererName = "dx9";
  io.BackendFlags |=
      BackendFlags_RendererHasVtxOffset; // We can honor the
                                         // DrawCmd::VtxOffset field,
                                         // allowing for large meshes.
  io.BackendFlags |=
      BackendFlags_RendererHasViewports; // We can create multi-viewports
                                         // on the Renderer side (optional)

  bd->pd3dDevice = device;
  bd->pd3dDevice->AddRef();

  if (io.ConfigFlags & ConfigFlags_ViewportsEnable)
    DX9_InitPlatformInterface();

  return true;
}

void DX9_Shutdown() {
  DX9_Data *bd = DX9_GetBackendData();
  ASSERT(bd != nullptr &&
         "No renderer backend to shutdown, or already shutdown?");
  IO &io = Gui::GetIO();

  DX9_ShutdownPlatformInterface();
  DX9_InvalidateDeviceObjects();
  if (bd->pd3dDevice) {
    bd->pd3dDevice->Release();
  }
  io.BackendRendererName = nullptr;
  io.BackendRendererUserData = nullptr;
  io.BackendFlags &=
      ~(BackendFlags_RendererHasVtxOffset | BackendFlags_RendererHasViewports);
  DELETE(bd);
}

static bool DX9_CreateFontsTexture() {
  // Build texture atlas
  IO &io = Gui::GetIO();
  DX9_Data *bd = DX9_GetBackendData();
  unsigned char *pixels;
  int width, height, bytes_per_pixel;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

  // Convert RGBA32 to BGRA32 (because RGBA32 is not well supported by DX9
  // devices)
#ifndef USE_BGRA_PACKED_COLOR
  if (io.Fonts->TexPixelsUseColors) {
    unsigned int *dst_start =
        (unsigned int *)Gui::MemAlloc((size_t)width * height * bytes_per_pixel);
    for (unsigned int *src = (unsigned int *)pixels, *dst = dst_start,
                      *dst_end = dst_start + (size_t)width * height;
         dst < dst_end; src++, dst++)
      *dst = COL_TO_DX9_ARGB(*src);
    pixels = (unsigned char *)dst_start;
  }
#endif

  // Upload texture to graphics system
  bd->FontTexture = nullptr;
  if (bd->pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC,
                                    D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                                    &bd->FontTexture, nullptr) < 0)
    return false;
  D3DLOCKED_RECT tex_locked_rect;
  if (bd->FontTexture->LockRect(0, &tex_locked_rect, nullptr, 0) != D3D_OK)
    return false;
  for (int y = 0; y < height; y++)
    memcpy((unsigned char *)tex_locked_rect.pBits +
               (size_t)tex_locked_rect.Pitch * y,
           pixels + (size_t)width * bytes_per_pixel * y,
           (size_t)width * bytes_per_pixel);
  bd->FontTexture->UnlockRect(0);

  // Store our identifier
  io.Fonts->SetTexID((TextureID)bd->FontTexture);

#ifndef USE_BGRA_PACKED_COLOR
  if (io.Fonts->TexPixelsUseColors)
    Gui::MemFree(pixels);
#endif

  return true;
}

bool DX9_CreateDeviceObjects() {
  DX9_Data *bd = DX9_GetBackendData();
  if (!bd || !bd->pd3dDevice)
    return false;
  if (!DX9_CreateFontsTexture())
    return false;
  DX9_CreateDeviceObjectsForPlatformWindows();
  return true;
}

void DX9_InvalidateDeviceObjects() {
  DX9_Data *bd = DX9_GetBackendData();
  if (!bd || !bd->pd3dDevice)
    return;
  if (bd->pVB) {
    bd->pVB->Release();
    bd->pVB = nullptr;
  }
  if (bd->pIB) {
    bd->pIB->Release();
    bd->pIB = nullptr;
  }
  if (bd->FontTexture) {
    bd->FontTexture->Release();
    bd->FontTexture = nullptr;
    Gui::GetIO().Fonts->SetTexID(0);
  } // We copied bd->pFontTextureView to io.Fonts->TexID so let's clear that as
    // well.
  DX9_InvalidateDeviceObjectsForPlatformWindows();
}

void DX9_NewFrame() {
  DX9_Data *bd = DX9_GetBackendData();
  ASSERT(bd != nullptr && "Did you call DX9_Init()?");

  if (!bd->FontTexture)
    DX9_CreateDeviceObjects();
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
struct DX9_ViewportData {
  IDirect3DSwapChain9 *SwapChain;
  D3DPRESENT_PARAMETERS d3dpp;

  DX9_ViewportData() {
    SwapChain = nullptr;
    ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
  }
  ~DX9_ViewportData() { ASSERT(SwapChain == nullptr); }
};

static void DX9_CreateWindow(Viewport *viewport) {
  DX9_Data *bd = DX9_GetBackendData();
  DX9_ViewportData *vd = NEW(DX9_ViewportData)();
  viewport->RendererUserData = vd;

  // PlatformHandleRaw should always be a HWND, whereas PlatformHandle might be
  // a higher-level handle (e.g. GLFWWindow*, SDL_Window*). Some backends will
  // leave PlatformHandleRaw == 0, in which case we assume PlatformHandle will
  // contain the HWND.
  HWND hwnd = viewport->PlatformHandleRaw ? (HWND)viewport->PlatformHandleRaw
                                          : (HWND)viewport->PlatformHandle;
  ASSERT(hwnd != 0);

  ZeroMemory(&vd->d3dpp, sizeof(D3DPRESENT_PARAMETERS));
  vd->d3dpp.Windowed = TRUE;
  vd->d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  vd->d3dpp.BackBufferWidth = (UINT)viewport->Size.x;
  vd->d3dpp.BackBufferHeight = (UINT)viewport->Size.y;
  vd->d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
  vd->d3dpp.hDeviceWindow = hwnd;
  vd->d3dpp.EnableAutoDepthStencil = FALSE;
  vd->d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
  vd->d3dpp.PresentationInterval =
      D3DPRESENT_INTERVAL_IMMEDIATE; // Present without vsync

  HRESULT hr =
      bd->pd3dDevice->CreateAdditionalSwapChain(&vd->d3dpp, &vd->SwapChain);
  UNUSED(hr);
  ASSERT(hr == D3D_OK);
  ASSERT(vd->SwapChain != nullptr);
}

static void DX9_DestroyWindow(Viewport *viewport) {
  // The main viewport (owned by the application) will always have
  // RendererUserData == 0 since we didn't create the data for it.
  if (DX9_ViewportData *vd = (DX9_ViewportData *)viewport->RendererUserData) {
    if (vd->SwapChain)
      vd->SwapChain->Release();
    vd->SwapChain = nullptr;
    ZeroMemory(&vd->d3dpp, sizeof(D3DPRESENT_PARAMETERS));
    DELETE(vd);
  }
  viewport->RendererUserData = nullptr;
}

static void DX9_SetWindowSize(Viewport *viewport, Vec2 size) {
  DX9_Data *bd = DX9_GetBackendData();
  DX9_ViewportData *vd = (DX9_ViewportData *)viewport->RendererUserData;
  if (vd->SwapChain) {
    vd->SwapChain->Release();
    vd->SwapChain = nullptr;
    vd->d3dpp.BackBufferWidth = (UINT)size.x;
    vd->d3dpp.BackBufferHeight = (UINT)size.y;
    HRESULT hr =
        bd->pd3dDevice->CreateAdditionalSwapChain(&vd->d3dpp, &vd->SwapChain);
    UNUSED(hr);
    ASSERT(hr == D3D_OK);
  }
}

static void DX9_RenderWindow(Viewport *viewport, void *) {
  DX9_Data *bd = DX9_GetBackendData();
  DX9_ViewportData *vd = (DX9_ViewportData *)viewport->RendererUserData;
  Vec4 clear_color = Vec4(0.0f, 0.0f, 0.0f, 1.0f);

  LPDIRECT3DSURFACE9 render_target = nullptr;
  LPDIRECT3DSURFACE9 last_render_target = nullptr;
  LPDIRECT3DSURFACE9 last_depth_stencil = nullptr;
  vd->SwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &render_target);
  bd->pd3dDevice->GetRenderTarget(0, &last_render_target);
  bd->pd3dDevice->GetDepthStencilSurface(&last_depth_stencil);
  bd->pd3dDevice->SetRenderTarget(0, render_target);
  bd->pd3dDevice->SetDepthStencilSurface(nullptr);

  if (!(viewport->Flags & ViewportFlags_NoRendererClear)) {
    D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(
        (int)(clear_color.x * 255.0f), (int)(clear_color.y * 255.0f),
        (int)(clear_color.z * 255.0f), (int)(clear_color.w * 255.0f));
    bd->pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET, clear_col_dx, 1.0f, 0);
  }

  DX9_RenderDrawData(viewport->DrawData);

  // Restore render target
  bd->pd3dDevice->SetRenderTarget(0, last_render_target);
  bd->pd3dDevice->SetDepthStencilSurface(last_depth_stencil);
  render_target->Release();
  last_render_target->Release();
  if (last_depth_stencil)
    last_depth_stencil->Release();
}

static void DX9_SwapBuffers(Viewport *viewport, void *) {
  DX9_ViewportData *vd = (DX9_ViewportData *)viewport->RendererUserData;
  HRESULT hr = vd->SwapChain->Present(nullptr, nullptr, vd->d3dpp.hDeviceWindow,
                                      nullptr, 0);
  // Let main application handle D3DERR_DEVICELOST by resetting the device.
  ASSERT(SUCCEEDED(hr) || hr == D3DERR_DEVICELOST);
}

static void DX9_InitPlatformInterface() {
  PlatformIO &platform_io = Gui::GetPlatformIO();
  platform_io.Renderer_CreateWindow = DX9_CreateWindow;
  platform_io.Renderer_DestroyWindow = DX9_DestroyWindow;
  platform_io.Renderer_SetWindowSize = DX9_SetWindowSize;
  platform_io.Renderer_RenderWindow = DX9_RenderWindow;
  platform_io.Renderer_SwapBuffers = DX9_SwapBuffers;
}

static void DX9_ShutdownPlatformInterface() { Gui::DestroyPlatformWindows(); }

static void DX9_CreateDeviceObjectsForPlatformWindows() {
  PlatformIO &platform_io = Gui::GetPlatformIO();
  for (int i = 1; i < platform_io.Viewports.Size; i++)
    if (!platform_io.Viewports[i]->RendererUserData)
      DX9_CreateWindow(platform_io.Viewports[i]);
}

static void DX9_InvalidateDeviceObjectsForPlatformWindows() {
  PlatformIO &platform_io = Gui::GetPlatformIO();
  for (int i = 1; i < platform_io.Viewports.Size; i++)
    if (platform_io.Viewports[i]->RendererUserData)
      DX9_DestroyWindow(platform_io.Viewports[i]);
}

//-----------------------------------------------------------------------------

#endif // #ifndef DISABLE
