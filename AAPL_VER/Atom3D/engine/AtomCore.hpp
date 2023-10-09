//
//  mtl_engine.h
//  Atom3D
//
//  Created by Jens Eckert on 10/4/23.
//

#ifndef mtl_engine_h
#define mtl_engine_h

#pragma once

#define GLFW_INCLUDE_NONE
#import <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#import <GLFW/glfw3native.h>

#include <Metal/Metal.hpp>
#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.hpp>
#include <QuartzCore/CAMetalLayer.h>
#include <QuartzCore/QuartzCore.hpp>

#include <simd/simd.h>

#include "VertexData.hpp"
#include "Texture.hpp"

#include "stb_image.h"

#include "AAPLMathUtilities.h"

#include <iostream>
#include <filesystem>

namespace Atom {
    
class AtomCore {
public:
    AtomCore() = default;
    
    void init();
    void run();
    void cleanup();
    void setSize(float, float);
private:
    void initDevice();
    void initWindow();
    
    void createTriangle();
    void createSquare();
    void createCube();
    
    void createDefaultLib();
    void createCommandQueue();
    void createRenderPipeline();
    
    void createBuffers();
    void createRenderPassDescriptor();
    void createDepthAndMSAATextures();
    
    void updateRenderPassDescriptor();
    
    void encodeRenderCommand(MTL::RenderCommandEncoder*);
    void draw();
    
    static void frameBufferSizeCallback(GLFWwindow*, int, int);
    void resizeFrameBuffer(int, int);
    
    MTL::Device* mDevice;
    GLFWwindow* mGlfwWindow;
    NSWindow* mMetalWindow;
    CAMetalLayer* mMetalLayer;
    CA::MetalDrawable* mMetalDrawable;
    
    MTL::Library* mDefaultLib;
    MTL::CommandQueue* mCommandQueue;
    MTL::CommandBuffer* mCommandBuffer;
    MTL::RenderPipelineState* mRenderPipelineState;
    
    MTL::Buffer* mVertexBuffer;
    MTL::Buffer* mTransformBuffer;
    MTL::DepthStencilState* mDepthStencilState;
    MTL::RenderPassDescriptor* mRenderPassDescriptor;
    MTL::Texture* mMSAARenderTargetTexture;
    MTL::Texture* mDepthTexture;
    
    Texture* mTexture;
    
    simd_float2 mViewSize = {800, 800};
        
    int mSampleCount = 4;
};
    
};

#endif /* mtl_engine_h */
