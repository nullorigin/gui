// gui: Renderer Backend for Metal
// This needs to be used along with a Platform Backend (e.g. OSX)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'MTLTexture' as TextureID. Read
//  the FAQ about TextureID! [X] Renderer: Large meshes support (64k+
//  vertices) with 16-bit indices. [X] Renderer: Multi-viewport support
//  (multiple windows). Enable with 'io.ConfigFlags |=
//  ConfigFlags_ViewportsEnable'.

#include "../gui.hpp"
#ifndef DISABLE
#include "metal.hpp"
#import <Metal/Metal.h>
#import <time.h>

// Forward Declarations
static void Metal_InitPlatformInterface();
static void Metal_ShutdownPlatformInterface();
static void Metal_CreateDeviceObjectsForPlatformWindows();
static void Metal_InvalidateDeviceObjectsForPlatformWindows();

#pragma mark - Support classes

// A wrapper around a MTLBuffer object that knows the last time it was reused
@interface MetalBuffer : NSObject
@property(nonatomic, strong) id<MTLBuffer> buffer;
@property(nonatomic, assign) double lastReuseTime;
- (instancetype)initWithBuffer:(id<MTLBuffer>)buffer;
@end

// An object that encapsulates the data necessary to uniquely identify a
// render pipeline state. These are used as cache keys.
@interface FramebufferDescriptor : NSObject <NSCopying>
@property(nonatomic, assign) unsigned long sampleCount;
@property(nonatomic, assign) MTLPixelFormat colorPixelFormat;
@property(nonatomic, assign) MTLPixelFormat depthPixelFormat;
@property(nonatomic, assign) MTLPixelFormat stencilPixelFormat;
- (instancetype)initWithRenderPassDescriptor:
    (MTLRenderPassDescriptor *)renderPassDescriptor;
@end

// A singleton that stores long-lived objects that are needed by the Metal
// renderer backend. Stores the render pipeline state cache and the default
// font texture, and manages the reusable buffer cache.
@interface MetalContext : NSObject
@property(nonatomic, strong) id<MTLDevice> device;
@property(nonatomic, strong) id<MTLDepthStencilState> depthStencilState;
@property(nonatomic, strong)
    FramebufferDescriptor *framebufferDescriptor; // framebuffer descriptor for
                                                  // current frame; transient
@property(nonatomic, strong)
    NSMutableDictionary *renderPipelineStateCache; // pipeline cache; keyed on
                                                   // framebuffer descriptors
@property(nonatomic, strong, nullable) id<MTLTexture> fontTexture;
@property(nonatomic, strong) NSMutableArray<MetalBuffer *> *bufferCache;
@property(nonatomic, assign) double lastBufferCachePurge;
- (MetalBuffer *)dequeueReusableBufferOfLength:(NSUInteger)length
                                        device:(id<MTLDevice>)device;
- (id<MTLRenderPipelineState>)
    renderPipelineStateForFramebufferDescriptor:
        (FramebufferDescriptor *)descriptor
                                         device:(id<MTLDevice>)device;
@end

struct Metal_Data {
  MetalContext *SharedMetalContext;

  Metal_Data() { memset(this, 0, sizeof(*this)); }
};

static Metal_Data *Metal_CreateBackendData() { return NEW(Metal_Data)(); }
static Metal_Data *Metal_GetBackendData() {
  return Gui::GetCurrentContext()
             ? (Metal_Data *)Gui::GetIO().BackendRendererUserData
             : nullptr;
}
static void Metal_DestroyBackendData() { DELETE(Metal_GetBackendData()); }

static inline CFTimeInterval GetMachAbsoluteTimeInSeconds() {
  return (CFTimeInterval)(double)(clock_gettime_nsec_np(CLOCK_UPTIME_RAW) /
                                  1e9);
}

#ifdef METAL_CPP

#pragma mark - Gui Metal C++ Backend API

bool Metal_Init(MTL::Device *device) {
  return Metal_Init((__bridge id<MTLDevice>)(device));
}

void Metal_NewFrame(MTL::RenderPassDescriptor *renderPassDescriptor) {
  Metal_NewFrame((__bridge MTLRenderPassDescriptor *)(renderPassDescriptor));
}

void Metal_RenderDrawData(DrawData *draw_data,
                          MTL::CommandBuffer *commandBuffer,
                          MTL::RenderCommandEncoder *commandEncoder) {
  Metal_RenderDrawData(draw_data,
                       (__bridge id<MTLCommandBuffer>)(commandBuffer),
                       (__bridge id<MTLRenderCommandEncoder>)(commandEncoder));
}

bool Metal_CreateFontsTexture(MTL::Device *device) {
  return Metal_CreateFontsTexture((__bridge id<MTLDevice>)(device));
}

bool Metal_CreateDeviceObjects(MTL::Device *device) {
  return Metal_CreateDeviceObjects((__bridge id<MTLDevice>)(device));
}

#endif // #ifdef METAL_CPP

#pragma mark - Gui Metal Backend API

bool Metal_Init(id<MTLDevice> device) {
  Metal_Data *bd = Metal_CreateBackendData();
  IO &io = Gui::GetIO();
  io.BackendRendererUserData = (void *)bd;
  io.BackendRendererName = "metal";
  io.BackendFlags |=
      BackendFlags_RendererHasVtxOffset; // We can honor the
                                         // DrawCmd::VtxOffset field,
                                         // allowing for large meshes.
  io.BackendFlags |=
      BackendFlags_RendererHasViewports; // We can create multi-viewports
                                         // on the Renderer side (optional)

  bd->SharedMetalContext = [[MetalContext alloc] init];
  bd->SharedMetalContext.device = device;

  if (io.ConfigFlags & ConfigFlags_ViewportsEnable)
    Metal_InitPlatformInterface();

  return true;
}

void Metal_Shutdown() {
  Metal_Data *bd = Metal_GetBackendData();
  assert(bd != nullptr &&
         "No renderer backend to shutdown, or already shutdown?");
  Metal_ShutdownPlatformInterface();
  Metal_DestroyDeviceObjects();
  Metal_DestroyBackendData();

  IO &io = Gui::GetIO();
  io.BackendRendererName = nullptr;
  io.BackendRendererUserData = nullptr;
  io.BackendFlags &=
      ~(BackendFlags_RendererHasVtxOffset | BackendFlags_RendererHasViewports);
}

void Metal_NewFrame(MTLRenderPassDescriptor *renderPassDescriptor) {
  Metal_Data *bd = Metal_GetBackendData();
  assert(bd->SharedMetalContext != nil &&
         "No Metal context. Did you call Metal_Init() ?");
  bd->SharedMetalContext.framebufferDescriptor = [[FramebufferDescriptor alloc]
      initWithRenderPassDescriptor:renderPassDescriptor];

  if (bd->SharedMetalContext.depthStencilState == nil)
    Metal_CreateDeviceObjects(bd->SharedMetalContext.device);
}

static void
Metal_SetupRenderState(DrawData *drawData, id<MTLCommandBuffer> commandBuffer,
                       id<MTLRenderCommandEncoder> commandEncoder,
                       id<MTLRenderPipelineState> renderPipelineState,
                       MetalBuffer *vertexBuffer, size_t vertexBufferOffset) {
  UNUSED(commandBuffer);
  Metal_Data *bd = Metal_GetBackendData();
  [commandEncoder setCullMode:MTLCullModeNone];
  [commandEncoder
      setDepthStencilState:bd->SharedMetalContext.depthStencilState];

  // Setup viewport, orthographic projection matrix
  // Our visible imgui space lies from draw_data->DisplayPos (top left) to
  // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is
  // typically (0,0) for single viewport apps.
  MTLViewport viewport = {
      .originX = 0.0,
      .originY = 0.0,
      .width = (double)(drawData->DisplaySize.x * drawData->FramebufferScale.x),
      .height =
          (double)(drawData->DisplaySize.y * drawData->FramebufferScale.y),
      .znear = 0.0,
      .zfar = 1.0};
  [commandEncoder setViewport:viewport];

  float L = drawData->DisplayPos.x;
  float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
  float T = drawData->DisplayPos.y;
  float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
  float N = (float)viewport.znear;
  float F = (float)viewport.zfar;
  const float ortho_projection[4][4] = {
      {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
      {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
      {0.0f, 0.0f, 1 / (F - N), 0.0f},
      {(R + L) / (L - R), (T + B) / (B - T), N / (F - N), 1.0f},
  };
  [commandEncoder setVertexBytes:&ortho_projection
                          length:sizeof(ortho_projection)
                         atIndex:1];

  [commandEncoder setRenderPipelineState:renderPipelineState];

  [commandEncoder setVertexBuffer:vertexBuffer.buffer offset:0 atIndex:0];
  [commandEncoder setVertexBufferOffset:vertexBufferOffset atIndex:0];
}

// Metal Render function.
void Metal_RenderDrawData(DrawData *drawData,
                          id<MTLCommandBuffer> commandBuffer,
                          id<MTLRenderCommandEncoder> commandEncoder) {
  Metal_Data *bd = Metal_GetBackendData();
  MetalContext *ctx = bd->SharedMetalContext;

  // Avoid rendering when minimized, scale coordinates for retina displays
  // (screen coordinates != framebuffer coordinates)
  int fb_width = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
  int fb_height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
  if (fb_width <= 0 || fb_height <= 0 || drawData->CmdListsCount == 0)
    return;

  // Try to retrieve a render pipeline state that is compatible with the
  // framebuffer config for this frame The hit rate for this cache should be
  // very near 100%.
  id<MTLRenderPipelineState> renderPipelineState =
      ctx.renderPipelineStateCache[ctx.framebufferDescriptor];
  if (renderPipelineState == nil) {
    // No luck; make a new render pipeline state
    renderPipelineState = [ctx
        renderPipelineStateForFramebufferDescriptor:ctx.framebufferDescriptor
                                             device:commandBuffer.device];

    // Cache render pipeline state for later reuse
    ctx.renderPipelineStateCache[ctx.framebufferDescriptor] =
        renderPipelineState;
  }

  size_t vertexBufferLength =
      (size_t)drawData->TotalVtxCount * sizeof(DrawVert);
  size_t indexBufferLength = (size_t)drawData->TotalIdxCount * sizeof(DrawIdx);
  MetalBuffer *vertexBuffer =
      [ctx dequeueReusableBufferOfLength:vertexBufferLength
                                  device:commandBuffer.device];
  MetalBuffer *indexBuffer =
      [ctx dequeueReusableBufferOfLength:indexBufferLength
                                  device:commandBuffer.device];

  Metal_SetupRenderState(drawData, commandBuffer, commandEncoder,
                         renderPipelineState, vertexBuffer, 0);

  // Will project scissor/clipping rectangles into framebuffer space
  Vec2 clip_off = drawData->DisplayPos; // (0,0) unless using multi-viewports
  Vec2 clip_scale = drawData->FramebufferScale; // (1,1) unless using retina
                                                // display which are often (2,2)

  // Render command lists
  size_t vertexBufferOffset = 0;
  size_t indexBufferOffset = 0;
  for (int n = 0; n < drawData->CmdListsCount; n++) {
    const DrawList *cmd_list = drawData->CmdLists[n];

    memcpy((char *)vertexBuffer.buffer.contents + vertexBufferOffset,
           cmd_list->VtxBuffer.Data,
           (size_t)cmd_list->VtxBuffer.Size * sizeof(DrawVert));
    memcpy((char *)indexBuffer.buffer.contents + indexBufferOffset,
           cmd_list->IdxBuffer.Data,
           (size_t)cmd_list->IdxBuffer.Size * sizeof(DrawIdx));

    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const DrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback) {
        // User callback, registered via DrawList::AddCallback()
        // (DrawCallback_ResetRenderState is a special callback value used by
        // the user to request the renderer to reset render state.)
        if (pcmd->UserCallback == DrawCallback_ResetRenderState)
          Metal_SetupRenderState(drawData, commandBuffer, commandEncoder,
                                 renderPipelineState, vertexBuffer,
                                 vertexBufferOffset);
        else
          pcmd->UserCallback(cmd_list, pcmd);
      } else {
        // Project scissor/clipping rectangles into framebuffer space
        Vec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                      (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
        Vec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                      (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

        // Clamp to viewport as setScissorRect() won't accept values that are
        // off bounds
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
        if (pcmd->ElemCount ==
            0) // drawIndexedPrimitives() validation doesn't accept this
          continue;

        // Apply scissor/clipping rectangle
        MTLScissorRect scissorRect = {
            .x = NSUInteger(clip_min.x),
            .y = NSUInteger(clip_min.y),
            .width = NSUInteger(clip_max.x - clip_min.x),
            .height = NSUInteger(clip_max.y - clip_min.y)};
        [commandEncoder setScissorRect:scissorRect];

        // Bind texture, Draw
        if (TextureID tex_id = pcmd->GetTexID())
          [commandEncoder setFragmentTexture:(__bridge id<MTLTexture>)(tex_id)
                                     atIndex:0];

        [commandEncoder
            setVertexBufferOffset:(vertexBufferOffset +
                                   pcmd->VtxOffset * sizeof(DrawVert))
                          atIndex:0];
        [commandEncoder
            drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                       indexCount:pcmd->ElemCount
                        indexType:sizeof(DrawIdx) == 2 ? MTLIndexTypeUInt16
                                                       : MTLIndexTypeUInt32
                      indexBuffer:indexBuffer.buffer
                indexBufferOffset:indexBufferOffset +
                                  pcmd->IdxOffset * sizeof(DrawIdx)];
      }
    }

    vertexBufferOffset += (size_t)cmd_list->VtxBuffer.Size * sizeof(DrawVert);
    indexBufferOffset += (size_t)cmd_list->IdxBuffer.Size * sizeof(DrawIdx);
  }

  [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer>) {
    dispatch_async(dispatch_get_main_queue(), ^{
      Metal_Data *bd = Metal_GetBackendData();
      if (bd != nullptr) {
        @synchronized(bd->SharedMetalContext.bufferCache) {
          [bd->SharedMetalContext.bufferCache addObject:vertexBuffer];
          [bd->SharedMetalContext.bufferCache addObject:indexBuffer];
        }
      }
    });
  }];
}

bool Metal_CreateFontsTexture(id<MTLDevice> device) {
  Metal_Data *bd = Metal_GetBackendData();
  IO &io = Gui::GetIO();

  // We are retrieving and uploading the font atlas as a 4-channels RGBA texture
  // here. In theory we could call GetTexDataAsAlpha8() and upload a 1-channel
  // texture to save on memory access bandwidth. However, using a shader
  // designed for 1-channel texture would make it less obvious to use the
  // TextureID facility to render users own textures. You can make that change
  // in your implementation.
  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  MTLTextureDescriptor *textureDescriptor = [MTLTextureDescriptor
      texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                   width:(NSUInteger)width
                                  height:(NSUInteger)height
                               mipmapped:NO];
  textureDescriptor.usage = MTLTextureUsageShaderRead;
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
  textureDescriptor.storageMode = MTLStorageModeManaged;
#else
  textureDescriptor.storageMode = MTLStorageModeShared;
#endif
  id<MTLTexture> texture = [device newTextureWithDescriptor:textureDescriptor];
  [texture
      replaceRegion:MTLRegionMake2D(0, 0, (NSUInteger)width, (NSUInteger)height)
        mipmapLevel:0
          withBytes:pixels
        bytesPerRow:(NSUInteger)width * 4];
  bd->SharedMetalContext.fontTexture = texture;
  io.Fonts->SetTexID((__bridge void *)bd->SharedMetalContext
                         .fontTexture); // TextureID == void*

  return (bd->SharedMetalContext.fontTexture != nil);
}

void Metal_DestroyFontsTexture() {
  Metal_Data *bd = Metal_GetBackendData();
  IO &io = Gui::GetIO();
  bd->SharedMetalContext.fontTexture = nil;
  io.Fonts->SetTexID(0);
}

bool Metal_CreateDeviceObjects(id<MTLDevice> device) {
  Metal_Data *bd = Metal_GetBackendData();
  MTLDepthStencilDescriptor *depthStencilDescriptor =
      [[MTLDepthStencilDescriptor alloc] init];
  depthStencilDescriptor.depthWriteEnabled = NO;
  depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
  bd->SharedMetalContext.depthStencilState =
      [device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
  Metal_CreateDeviceObjectsForPlatformWindows();
  Metal_CreateFontsTexture(device);

  return true;
}

void Metal_DestroyDeviceObjects() {
  Metal_Data *bd = Metal_GetBackendData();
  Metal_DestroyFontsTexture();
  Metal_InvalidateDeviceObjectsForPlatformWindows();
  [bd->SharedMetalContext.renderPipelineStateCache removeAllObjects];
}

#pragma mark - Multi-viewport support

#import <QuartzCore/CAMetalLayer.h>

#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#endif

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the back-end to create
// and handle multiple viewports simultaneously. If you are new to gui or
// creating a new binding for gui, it is recommended that you completely
// ignore this section first..
//--------------------------------------------------------------------------------------------------------

struct ViewportDataMetal {
  CAMetalLayer *MetalLayer;
  id<MTLCommandQueue> CommandQueue;
  MTLRenderPassDescriptor *RenderPassDescriptor;
  void *Handle = nullptr;
  bool FirstFrame = true;
};

static void Metal_CreateWindow(Viewport *viewport) {
  Metal_Data *bd = Metal_GetBackendData();
  ViewportDataMetal *data = NEW(ViewportDataMetal)();
  viewport->RendererUserData = data;

  // PlatformHandleRaw should always be a NSWindow*, whereas PlatformHandle
  // might be a higher-level handle (e.g. GLFWWindow*, SDL_Window*). Some
  // back-ends will leave PlatformHandleRaw == 0, in which case we assume
  // PlatformHandle will contain the NSWindow*.
  void *handle = viewport->PlatformHandleRaw ? viewport->PlatformHandleRaw
                                             : viewport->PlatformHandle;
  assert(handle != nullptr);

  id<MTLDevice> device = bd->SharedMetalContext.device;
  CAMetalLayer *layer = [CAMetalLayer layer];
  layer.device = device;
  layer.framebufferOnly = YES;
  layer.pixelFormat =
      bd->SharedMetalContext.framebufferDescriptor.colorPixelFormat;
#if TARGET_OS_OSX
  NSWindow *window = (__bridge NSWindow *)handle;
  NSView *view = window.contentView;
  view.layer = layer;
  view.wantsLayer = YES;
#endif
  data->MetalLayer = layer;
  data->CommandQueue = [device newCommandQueue];
  data->RenderPassDescriptor = [[MTLRenderPassDescriptor alloc] init];
  data->Handle = handle;
}

static void Metal_DestroyWindow(Viewport *viewport) {
  // The main viewport (owned by the application) will always have
  // RendererUserData == 0 since we didn't create the data for it.
  if (ViewportDataMetal *data = (ViewportDataMetal *)viewport->RendererUserData)
    DELETE(data);
  viewport->RendererUserData = nullptr;
}

inline static CGSize MakeScaledSize(CGSize size, CGFloat scale) {
  return CGSizeMake(size.width * scale, size.height * scale);
}

static void Metal_SetWindowSize(Viewport *viewport, Vec2 size) {
  ViewportDataMetal *data = (ViewportDataMetal *)viewport->RendererUserData;
  data->MetalLayer.drawableSize =
      MakeScaledSize(CGSizeMake(size.x, size.y), viewport->DpiScale);
}

static void Metal_RenderWindow(Viewport *viewport, void *) {
  ViewportDataMetal *data = (ViewportDataMetal *)viewport->RendererUserData;

#if TARGET_OS_OSX
  void *handle = viewport->PlatformHandleRaw ? viewport->PlatformHandleRaw
                                             : viewport->PlatformHandle;
  NSWindow *window = (__bridge NSWindow *)handle;

  // Always render the first frame, regardless of occlusionState, to avoid an
  // initial flicker
  if ((window.occlusionState & NSWindowOcclusionStateVisible) == 0 &&
      !data->FirstFrame) {
    // Do not render windows which are completely occluded. Calling
    // -[CAMetalLayer nextDrawable] will hang for approximately 1 second if the
    // Metal layer is completely occluded.
    return;
  }
  data->FirstFrame = false;

  viewport->DpiScale = (float)window.backingScaleFactor;
  if (data->MetalLayer.contentsScale != viewport->DpiScale) {
    data->MetalLayer.contentsScale = viewport->DpiScale;
    data->MetalLayer.drawableSize =
        MakeScaledSize(window.frame.size, viewport->DpiScale);
  }
  viewport->DrawData->FramebufferScale =
      Vec2(viewport->DpiScale, viewport->DpiScale);
#endif

  id<CAMetalDrawable> drawable = [data->MetalLayer nextDrawable];
  if (drawable == nil)
    return;

  MTLRenderPassDescriptor *renderPassDescriptor = data->RenderPassDescriptor;
  renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
  renderPassDescriptor.colorAttachments[0].clearColor =
      MTLClearColorMake(0, 0, 0, 0);
  if ((viewport->Flags & ViewportFlags_NoRendererClear) == 0)
    renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;

  id<MTLCommandBuffer> commandBuffer = [data->CommandQueue commandBuffer];
  id<MTLRenderCommandEncoder> renderEncoder =
      [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
  Metal_RenderDrawData(viewport->DrawData, commandBuffer, renderEncoder);
  [renderEncoder endEncoding];

  [commandBuffer presentDrawable:drawable];
  [commandBuffer commit];
}

static void Metal_InitPlatformInterface() {
  PlatformIO &platform_io = Gui::GetPlatformIO();
  platform_io.Renderer_CreateWindow = Metal_CreateWindow;
  platform_io.Renderer_DestroyWindow = Metal_DestroyWindow;
  platform_io.Renderer_SetWindowSize = Metal_SetWindowSize;
  platform_io.Renderer_RenderWindow = Metal_RenderWindow;
}

static void Metal_ShutdownPlatformInterface() { Gui::DestroyPlatformWindows(); }

static void Metal_CreateDeviceObjectsForPlatformWindows() {
  PlatformIO &platform_io = Gui::GetPlatformIO();
  for (int i = 1; i < platform_io.Viewports.Size; i++)
    if (!platform_io.Viewports[i]->RendererUserData)
      Metal_CreateWindow(platform_io.Viewports[i]);
}

static void Metal_InvalidateDeviceObjectsForPlatformWindows() {
  PlatformIO &platform_io = Gui::GetPlatformIO();
  for (int i = 1; i < platform_io.Viewports.Size; i++)
    if (platform_io.Viewports[i]->RendererUserData)
      Metal_DestroyWindow(platform_io.Viewports[i]);
}

#pragma mark - MetalBuffer implementation

@implementation MetalBuffer
- (instancetype)initWithBuffer:(id<MTLBuffer>)buffer {
  if ((self = [super init])) {
    _buffer = buffer;
    _lastReuseTime = GetMachAbsoluteTimeInSeconds();
  }
  return self;
}
@end

#pragma mark - FramebufferDescriptor implementation

@implementation FramebufferDescriptor
- (instancetype)initWithRenderPassDescriptor:
    (MTLRenderPassDescriptor *)renderPassDescriptor {
  if ((self = [super init])) {
    _sampleCount = renderPassDescriptor.colorAttachments[0].texture.sampleCount;
    _colorPixelFormat =
        renderPassDescriptor.colorAttachments[0].texture.pixelFormat;
    _depthPixelFormat =
        renderPassDescriptor.depthAttachment.texture.pixelFormat;
    _stencilPixelFormat =
        renderPassDescriptor.stencilAttachment.texture.pixelFormat;
  }
  return self;
}

- (nonnull id)copyWithZone:(nullable NSZone *)zone {
  FramebufferDescriptor *copy =
      [[FramebufferDescriptor allocWithZone:zone] init];
  copy.sampleCount = self.sampleCount;
  copy.colorPixelFormat = self.colorPixelFormat;
  copy.depthPixelFormat = self.depthPixelFormat;
  copy.stencilPixelFormat = self.stencilPixelFormat;
  return copy;
}

- (NSUInteger)hash {
  NSUInteger sc = _sampleCount & 0x3;
  NSUInteger cf = _colorPixelFormat & 0x3FF;
  NSUInteger df = _depthPixelFormat & 0x3FF;
  NSUInteger sf = _stencilPixelFormat & 0x3FF;
  NSUInteger hash = (sf << 22) | (df << 12) | (cf << 2) | sc;
  return hash;
}

- (BOOL)isEqual:(id)object {
  FramebufferDescriptor *other = object;
  if (![other isKindOfClass:[FramebufferDescriptor class]])
    return NO;
  return other.sampleCount == self.sampleCount &&
         other.colorPixelFormat == self.colorPixelFormat &&
         other.depthPixelFormat == self.depthPixelFormat &&
         other.stencilPixelFormat == self.stencilPixelFormat;
}

@end

#pragma mark - MetalContext implementation

@implementation MetalContext
- (instancetype)init {
  if ((self = [super init])) {
    self.renderPipelineStateCache = [NSMutableDictionary dictionary];
    self.bufferCache = [NSMutableArray array];
    _lastBufferCachePurge = GetMachAbsoluteTimeInSeconds();
  }
  return self;
}

- (MetalBuffer *)dequeueReusableBufferOfLength:(NSUInteger)length
                                        device:(id<MTLDevice>)device {
  uint64_t now = GetMachAbsoluteTimeInSeconds();

  @synchronized(self.bufferCache) {
    // Purge old buffers that haven't been useful for a while
    if (now - self.lastBufferCachePurge > 1.0) {
      NSMutableArray *survivors = [NSMutableArray array];
      for (MetalBuffer *candidate in self.bufferCache)
        if (candidate.lastReuseTime > self.lastBufferCachePurge)
          [survivors addObject:candidate];
      self.bufferCache = [survivors mutableCopy];
      self.lastBufferCachePurge = now;
    }

    // See if we have a buffer we can reuse
    MetalBuffer *bestCandidate = nil;
    for (MetalBuffer *candidate in self.bufferCache)
      if (candidate.buffer.length >= length &&
          (bestCandidate == nil ||
           bestCandidate.lastReuseTime > candidate.lastReuseTime))
        bestCandidate = candidate;

    if (bestCandidate != nil) {
      [self.bufferCache removeObject:bestCandidate];
      bestCandidate.lastReuseTime = now;
      return bestCandidate;
    }
  }

  // No luck; make a new buffer
  id<MTLBuffer> backing =
      [device newBufferWithLength:length options:MTLResourceStorageModeShared];
  return [[MetalBuffer alloc] initWithBuffer:backing];
}

// Bilinear sampling is required by default. Set 'io.Fonts->Flags |=
// FontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to
// allow point/nearest sampling.
- (id<MTLRenderPipelineState>)
    renderPipelineStateForFramebufferDescriptor:
        (FramebufferDescriptor *)descriptor
                                         device:(id<MTLDevice>)device {
  NSError *error = nil;

  NSString *shaderSource =
      @""
       "#include <metal_stdlib>\n"
       "using namespace metal;\n"
       "\n"
       "struct Uniforms {\n"
       "    float4x4 projectionMatrix;\n"
       "};\n"
       "\n"
       "struct VertexIn {\n"
       "    float2 position  [[attribute(0)]];\n"
       "    float2 texCoords [[attribute(1)]];\n"
       "    uchar4 color     [[attribute(2)]];\n"
       "};\n"
       "\n"
       "struct VertexOut {\n"
       "    float4 position [[position]];\n"
       "    float2 texCoords;\n"
       "    float4 color;\n"
       "};\n"
       "\n"
       "vertex VertexOut vertex_main(VertexIn in                 "
       "[[stage_in]],\n"
       "                             constant Uniforms &uniforms "
       "[[buffer(1)]]) {\n"
       "    VertexOut out;\n"
       "    out.position = uniforms.projectionMatrix * float4(in.position, 0, "
       "1);\n"
       "    out.texCoords = in.texCoords;\n"
       "    out.color = float4(in.color) / float4(255.0);\n"
       "    return out;\n"
       "}\n"
       "\n"
       "fragment half4 fragment_main(VertexOut in [[stage_in]],\n"
       "                             texture2d<half, access::sample> texture "
       "[[texture(0)]]) {\n"
       "    constexpr sampler linearSampler(coord::normalized, "
       "min_filter::linear, mag_filter::linear, mip_filter::linear);\n"
       "    half4 texColor = texture.sample(linearSampler, in.texCoords);\n"
       "    return half4(in.color) * texColor;\n"
       "}\n";

  id<MTLLibrary> library = [device newLibraryWithSource:shaderSource
                                                options:nil
                                                  error:&error];
  if (library == nil) {
    NSLog(@"Error: failed to create Metal library: %@", error);
    return nil;
  }

  id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertex_main"];
  id<MTLFunction> fragmentFunction =
      [library newFunctionWithName:@"fragment_main"];

  if (vertexFunction == nil || fragmentFunction == nil) {
    NSLog(@"Error: failed to find Metal shader functions in library: %@",
          error);
    return nil;
  }

  MTLVertexDescriptor *vertexDescriptor =
      [MTLVertexDescriptor vertexDescriptor];
  vertexDescriptor.attributes[0].offset = offsetof(DrawVert, pos);
  vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2; // position
  vertexDescriptor.attributes[0].bufferIndex = 0;
  vertexDescriptor.attributes[1].offset = offsetof(DrawVert, uv);
  vertexDescriptor.attributes[1].format = MTLVertexFormatFloat2; // texCoords
  vertexDescriptor.attributes[1].bufferIndex = 0;
  vertexDescriptor.attributes[2].offset = offsetof(DrawVert, col);
  vertexDescriptor.attributes[2].format = MTLVertexFormatUChar4; // color
  vertexDescriptor.attributes[2].bufferIndex = 0;
  vertexDescriptor.layouts[0].stepRate = 1;
  vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
  vertexDescriptor.layouts[0].stride = sizeof(DrawVert);

  MTLRenderPipelineDescriptor *pipelineDescriptor =
      [[MTLRenderPipelineDescriptor alloc] init];
  pipelineDescriptor.vertexFunction = vertexFunction;
  pipelineDescriptor.fragmentFunction = fragmentFunction;
  pipelineDescriptor.vertexDescriptor = vertexDescriptor;
  pipelineDescriptor.rasterSampleCount = self.framebufferDescriptor.sampleCount;
  pipelineDescriptor.colorAttachments[0].pixelFormat =
      self.framebufferDescriptor.colorPixelFormat;
  pipelineDescriptor.colorAttachments[0].blendingEnabled = YES;
  pipelineDescriptor.colorAttachments[0].rgbBlendOperation =
      MTLBlendOperationAdd;
  pipelineDescriptor.colorAttachments[0].sourceRGBBlendFactor =
      MTLBlendFactorSourceAlpha;
  pipelineDescriptor.colorAttachments[0].destinationRGBBlendFactor =
      MTLBlendFactorOneMinusSourceAlpha;
  pipelineDescriptor.colorAttachments[0].alphaBlendOperation =
      MTLBlendOperationAdd;
  pipelineDescriptor.colorAttachments[0].sourceAlphaBlendFactor =
      MTLBlendFactorOne;
  pipelineDescriptor.colorAttachments[0].destinationAlphaBlendFactor =
      MTLBlendFactorOneMinusSourceAlpha;
  pipelineDescriptor.depthAttachmentPixelFormat =
      self.framebufferDescriptor.depthPixelFormat;
  pipelineDescriptor.stencilAttachmentPixelFormat =
      self.framebufferDescriptor.stencilPixelFormat;

  id<MTLRenderPipelineState> renderPipelineState =
      [device newRenderPipelineStateWithDescriptor:pipelineDescriptor
                                             error:&error];
  if (error != nil)
    NSLog(@"Error: failed to create Metal pipeline state: %@", error);

  return renderPipelineState;
}

@end

//-----------------------------------------------------------------------------

#endif // #ifndef DISABLE
