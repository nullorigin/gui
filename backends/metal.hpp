// gui: Renderer Backend for Metal
// This needs to be used along with a Platform Backend (e.g. OSX)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'MTLTexture' as TextureID. Read
//  the FAQ about TextureID! [X] Renderer: Large meshes support (64k+
//  vertices) with 16-bit indices. [X] Renderer: Multi-viewport support
//  (multiple windows). Enable with 'io.ConfigFlags |=
//  ConfigFlags_ViewportsEnable'.
//
#include "../gui.hpp" // API
#ifndef DISABLE

//-----------------------------------------------------------------------------
// ObjC API
//-----------------------------------------------------------------------------

#ifdef __OBJC__

@class MTLRenderPassDescriptor;
@protocol MTLDevice
, MTLCommandBuffer, MTLRenderCommandEncoder;

API bool Metal_Init(id<MTLDevice> device);
API void Metal_Shutdown();
API void Metal_NewFrame(MTLRenderPassDescriptor *renderPassDescriptor);
API void Metal_RenderDrawData(DrawData *drawData,
                              id<MTLCommandBuffer> commandBuffer,
                              id<MTLRenderCommandEncoder> commandEncoder);

// Called by Init/NewFrame/Shutdown
API bool Metal_CreateFontsTexture(id<MTLDevice> device);
API void Metal_DestroyFontsTexture();
API bool Metal_CreateDeviceObjects(id<MTLDevice> device);
API void Metal_DestroyDeviceObjects();

#endif

//-----------------------------------------------------------------------------
// C++ API
//-----------------------------------------------------------------------------

// Enable Metal C++ binding support with '#define METAL_CPP' in your
// config.hpp file More info about using Metal from C++:
// https://developer.apple.com/metal/cpp/

#ifdef METAL_CPP
#include <Metal/Metal.hpp>
#ifndef __OBJC__

API bool Metal_Init(MTL::Device *device);
API void Metal_Shutdown();
API void Metal_NewFrame(MTL::RenderPassDescriptor *renderPassDescriptor);
API void Metal_RenderDrawData(DrawData *draw_data,
                              MTL::CommandBuffer *commandBuffer,
                              MTL::RenderCommandEncoder *commandEncoder);

// Called by Init/NewFrame/Shutdown
API bool Metal_CreateFontsTexture(MTL::Device *device);
API void Metal_DestroyFontsTexture();
API bool Metal_CreateDeviceObjects(MTL::Device *device);
API void Metal_DestroyDeviceObjects();

#endif
#endif

//-----------------------------------------------------------------------------

#endif // #ifndef DISABLE
