//
//  mtl_engine.mm
//  Atom3D
//
//  Created by Jens Eckert on 10/4/23.
//

#include "AtomCore.hpp"

namespace Atom {
    
    // Public Functions
    void AtomCore::init() {
        initDevice();
        initWindow();
        
        createCube();
        createBuffers();
        createDefaultLib();
        createCommandQueue();
        createRenderPipeline();
        createDepthAndMSAATextures();
        createRenderPassDescriptor();
    }
    
    void AtomCore::run() {
        while (!glfwWindowShouldClose(mGlfwWindow)) {
            @autoreleasepool {
                mMetalDrawable = (__bridge CA::MetalDrawable*)[mMetalLayer nextDrawable];
                draw();
            }
            
            glfwPollEvents();
        }
    }
    
    void AtomCore::cleanup() {
        glfwTerminate();
        mTransformBuffer->release();
        mMSAARenderTargetTexture-> release();
        mDepthTexture->release();
        mRenderPassDescriptor->release();
        mDevice->release();
        delete mTexture;
    }
    
    void AtomCore::setSize(float x, float y) {
        mViewSize = {x, y};
    }
    
    
    // Init functions
    void AtomCore::initDevice() {
        mDevice = MTL::CreateSystemDefaultDevice();
    }
    
    void AtomCore::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        mGlfwWindow = glfwCreateWindow(mViewSize.x, mViewSize.y, "Atom3D", nullptr, nullptr);
        
        if (!mGlfwWindow) {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        
        int w, h;
        glfwGetFramebufferSize(mGlfwWindow, &w, &h);
        glfwSetWindowUserPointer(mGlfwWindow, this);
        glfwSetFramebufferSizeCallback(mGlfwWindow, frameBufferSizeCallback);
        
        mMetalWindow = glfwGetCocoaWindow(mGlfwWindow);
        mMetalLayer = [CAMetalLayer layer];
        mMetalLayer.device = (__bridge id<MTLDevice>)mDevice;
        mMetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        mMetalLayer.drawableSize = CGSizeMake(w, h);
        mMetalWindow.contentView.layer = mMetalLayer;
        mMetalWindow.contentView.wantsLayer = YES;
        
        mMetalDrawable = (__bridge CA::MetalDrawable*) [mMetalLayer nextDrawable];
        
    }
    
    
    // Meshes
    void AtomCore::createTriangle() {
        simd::float3 verts[] = {
            {-0.5f, -0.5f, 0.0f},
            { 0.5f, -0.5f, 0.0f},
            { 0.0f,  0.5f, 0.0f}
        };
        
        mVertexBuffer = mDevice->newBuffer(&verts, sizeof verts, MTL::ResourceStorageModeShared);
    }
    
    void AtomCore::createSquare() {
        VertexData verts[] {
            {{-0.5, -0.5,  0.5, 1.0f}, {0.0f, 0.0f}},
            {{-0.5,  0.5,  0.5, 1.0f}, {0.0f, 1.0f}},
            {{ 0.5,  0.5,  0.5, 1.0f}, {1.0f, 1.0f}},
            {{-0.5, -0.5,  0.5, 1.0f}, {0.0f, 0.0f}},
            {{ 0.5,  0.5,  0.5, 1.0f}, {1.0f, 1.0f}},
            {{ 0.5, -0.5,  0.5, 1.0f}, {1.0f, 0.0f}}
        };
        
        mVertexBuffer = mDevice->newBuffer(&verts, sizeof verts, MTL::ResourceStorageModeShared);
        
        mTexture = new Texture("engine/assets/NickWiz.png", mDevice);
    }
    
    void AtomCore::createCube() {
        VertexData cubeVertices[] = {
            // Back face
            {{-0.5, -0.5,  0.5, 1.0f},  {1.0f, 0.0f}}, // bottom-left  4
            {{ 0.5, -0.5,  0.5, 1.0f},  {0.0f, 0.0f}}, // bottom-right 6
            {{-0.5,  0.5,  0.5, 1.0f},  {1.0f, 1.0f}}, // top-left     5
            {{ 0.5, -0.5,  0.5, 1.0f},  {0.0f, 0.0f}}, // bottom-right 6
            {{ 0.5,  0.5,  0.5, 1.0f},  {0.0f, 1.0f}}, // top-right    7
            {{-0.5,  0.5,  0.5, 1.0f},  {1.0f, 1.0f}}, // top-left     5
                                                       // Right face
            {{ 0.5, -0.5,  0.5, 1.0f},  {1.0f, 0.0f}}, // bottom-right 6
            {{ 0.5, -0.5, -0.5, 1.0f},  {0.0f, 0.0f}}, // bottom-right 2
            {{ 0.5,  0.5,  0.5, 1.0f},  {1.0f, 1.0f}}, // top-right    7
            {{ 0.5, -0.5, -0.5, 1.0f},  {0.0f, 0.0f}}, // bottom-right 2
            {{ 0.5,  0.5, -0.5, 1.0f},  {0.0f, 1.0f}}, // bottom-right 6
            {{ 0.5,  0.5,  0.5, 1.0f},  {1.0f, 1.0f}}, // top-right    3
                                                       // Front face
            {{ 0.5, -0.5, -0.5, 1.0f},  {1.0f, 0.0f}}, // bottom-right 2
            {{-0.5, -0.5, -0.5, 1.0f},  {0.0f, 0.0f}}, // bottom-left  0
            {{ 0.5,  0.5, -0.5, 1.0f},  {1.0f, 1.0f}}, // top-right    3
            {{-0.5, -0.5, -0.5, 1.0f},  {0.0f, 0.0f}}, // bottom-left  0
            {{-0.5,  0.5, -0.5, 1.0f},  {0.0f, 1.0f}}, // top-left     1
            {{ 0.5,  0.5, -0.5, 1.0f},  {1.0f, 1.0f}}, // top-right    3
                                                       // Left face
            {{-0.5, -0.5, -0.5, 1.0f},  {1.0f, 0.0f}}, // bottom-left  0
            {{-0.5, -0.5,  0.5, 1.0f},  {0.0f, 0.0f}}, // top-left     1
            {{-0.5,  0.5, -0.5, 1.0f},  {1.0f, 1.0f}}, // top-left     5
            {{-0.5, -0.5,  0.5, 1.0f},  {0.0f, 0.0f}}, // bottom-left  0
            {{-0.5,  0.5,  0.5, 1.0f},  {0.0f, 1.0f}}, // top-left     5
            {{-0.5,  0.5, -0.5, 1.0f},  {1.0f, 1.0f}}, // bottom-left  4
                                                       // Top face
            {{ 0.5,  0.5, -0.5, 1.0f},  {1.0f, 0.0f}}, // top-left     5
            {{-0.5,  0.5, -0.5, 1.0f},  {0.0f, 0.0f}}, // top-left     1
            {{ 0.5,  0.5,  0.5, 1.0f},  {1.0f, 1.0f}}, // top-right    3
            {{-0.5,  0.5, -0.5, 1.0f},  {0.0f, 0.0f}}, // top-left     5
            {{-0.5,  0.5,  0.5, 1.0f},  {0.0f, 1.0f}}, // top-right    3
            {{ 0.5,  0.5,  0.5, 1.0f},  {1.0f, 1.0f}}, // top-right    7
                                                       // Bottom face
            {{ 0.5, -0.5,  0.5, 1.0f},  {1.0f, 0.0f}}, // bottom-left  0
            {{-0.5, -0.5,  0.5, 1.0f},  {0.0f, 0.0f}}, // bottom-left  4
            {{ 0.5, -0.5, -0.5, 1.0f},  {1.0f, 1.0f}}, // bottom-right 6
            {{-0.5, -0.5,  0.5, 1.0f},  {0.0f, 0.0f}}, // bottom-left  0
            {{-0.5, -0.5, -0.5, 1.0f},  {0.0f, 1.0f}}, // bottom-right 6
            {{ 0.5, -0.5, -0.5, 1.0f},  {1.0f, 1.0f}}  // bottom-right 2
        };
        
        mVertexBuffer = mDevice->newBuffer(&cubeVertices, sizeof cubeVertices, MTL::ResourceStorageModeShared);
        
        mTexture = new Texture("engine/assets/mc_grass.jpeg", mDevice);
    }
    
    void AtomCore::createBuffers() {
        mTransformBuffer = mDevice->newBuffer(sizeof(TransformData), MTL::ResourceStorageModeShared);
    }
    
    
    // One time setup.
    void AtomCore::createDefaultLib() {
        mDefaultLib = mDevice->newDefaultLibrary();
        if (!mDefaultLib) {
            std::cerr << "Failed to load Default Library.\n";
            std::exit(-1);
        }
    }
    
    void AtomCore::createCommandQueue() {
        mCommandQueue = mDevice->newCommandQueue();
        if (!mCommandQueue) {
            std::cerr << "Failed to initialize command queue";
            std::exit(-1);
        }
    }
    
    void AtomCore::createRenderPipeline() {
        // Create shader functions
        auto vertexShader = mDefaultLib->newFunction(NS::String::string("vertexShader", NS::ASCIIStringEncoding));
        assert(vertexShader);
        auto fragShader = mDefaultLib->newFunction(NS::String::string("fragmentShader", NS::ASCIIStringEncoding));
        assert(fragShader);
        
        auto renderPipeDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
        renderPipeDescriptor->setLabel(NS::String::string("Square Pipeline Descriptor", NS::ASCIIStringEncoding));
        renderPipeDescriptor->setVertexFunction(vertexShader);
        renderPipeDescriptor->setFragmentFunction(fragShader);
        
        assert(renderPipeDescriptor);
        
        renderPipeDescriptor->colorAttachments()->object(0)->setPixelFormat((MTL::PixelFormat)mMetalLayer.pixelFormat);
        
        renderPipeDescriptor->setSampleCount(mSampleCount);
        renderPipeDescriptor->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);
        
        NS::Error* error;
        mRenderPipelineState = mDevice->newRenderPipelineState(renderPipeDescriptor, &error);
        
        if (mRenderPipelineState == nil) {
            std::cout << "Error creating mRenderPipelineState: " << error << std::endl;
            std::exit(-1);
        }
        
        auto depthStencilDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
        depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunctionLessEqual);
        depthStencilDescriptor->setDepthWriteEnabled(true);
        mDepthStencilState = mDevice->newDepthStencilState(depthStencilDescriptor);
        
        renderPipeDescriptor->release();
        vertexShader->release();
        fragShader->release();
        depthStencilDescriptor->release();
    }
    
    void AtomCore::createDepthAndMSAATextures() {
        // MSAA setup
        auto msaaTextureDescriptor = MTL::TextureDescriptor::alloc()->init();
        msaaTextureDescriptor->setTextureType(MTL::TextureType2DMultisample);
        msaaTextureDescriptor->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
        msaaTextureDescriptor->setWidth(mMetalLayer.drawableSize.width);
        msaaTextureDescriptor->setHeight(mMetalLayer.drawableSize.height);
        msaaTextureDescriptor->setSampleCount(mSampleCount);
        msaaTextureDescriptor->setUsage(MTL::TextureUsageRenderTarget);
        
        mMSAARenderTargetTexture = mDevice->newTexture(msaaTextureDescriptor);
        
        // Depth stencil setup
        auto depthStencilDescriptor = MTL::TextureDescriptor::alloc()->init();
        depthStencilDescriptor->setTextureType(MTL::TextureType2DMultisample);
        depthStencilDescriptor->setPixelFormat(MTL::PixelFormatDepth32Float);
        depthStencilDescriptor->setWidth(mMetalLayer.drawableSize.width);
        depthStencilDescriptor->setHeight(mMetalLayer.drawableSize.height);
        depthStencilDescriptor->setUsage(MTL::TextureUsageRenderTarget);
        depthStencilDescriptor->setSampleCount(mSampleCount);
        
        mDepthTexture = mDevice->newTexture(depthStencilDescriptor);
        
        // Kill the children
        msaaTextureDescriptor->release();
        depthStencilDescriptor->release();
    }
    
    void AtomCore::createRenderPassDescriptor() {
        mRenderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();
        
        auto colorAttachment = mRenderPassDescriptor->colorAttachments()->object(0);
        auto depthAttachment = mRenderPassDescriptor->depthAttachment();
        
        colorAttachment->setTexture(mMSAARenderTargetTexture);
        colorAttachment->setResolveTexture(mMetalDrawable->texture());
        colorAttachment->setLoadAction(MTL::LoadActionClear);
        colorAttachment->setClearColor(MTL::ClearColor(0.2f, 0.5f, 0.1f, 1.0f));
        colorAttachment->setStoreAction(MTL::StoreActionMultisampleResolve);
        
        depthAttachment->setTexture(mDepthTexture);
        depthAttachment->setLoadAction(MTL::LoadActionClear);
        depthAttachment->setStoreAction(MTL::StoreActionDontCare);
        depthAttachment->setClearDepth(1.0f);
        
        // release?
    }
    
    void AtomCore::updateRenderPassDescriptor() {
        mRenderPassDescriptor->colorAttachments()->object(0)->setTexture(mMSAARenderTargetTexture);
        mRenderPassDescriptor->colorAttachments()->object(0)->setResolveTexture(mMetalDrawable->texture());
        mRenderPassDescriptor->depthAttachment()->setTexture(mDepthTexture);
    }
    
    
    // Actual Rendering Code/Frequently Called.
    void AtomCore::draw() {
        mCommandBuffer = mCommandQueue->commandBuffer();
        
        updateRenderPassDescriptor();
        
        MTL::RenderCommandEncoder* rce = mCommandBuffer->renderCommandEncoder(mRenderPassDescriptor);
        encodeRenderCommand(rce);
        rce->endEncoding();
        
        mCommandBuffer->presentDrawable(mMetalDrawable);
        mCommandBuffer->commit();
        mCommandBuffer->waitUntilCompleted();
    }
    
    void AtomCore::encodeRenderCommand(MTL::RenderCommandEncoder* rce) {
        matrix_float4x4 transMat = matrix4x4_translation(0, 0, 0);
        
        float angleInDeg = glfwGetTime() / 2 * 90;
        float angleInRad = angleInDeg * M_PI / 180;
        matrix_float4x4 rotMat = matrix4x4_rotation(angleInRad, 0, -1, 0);
        
        matrix_float4x4 modelMat = matrix_identity_float4x4;
        modelMat = simd_mul(transMat, rotMat);
        
        matrix_float4x4 viewMat = matrix4x4_translation(0, 0, 2);
        
        float aspectRatio = (mMetalLayer.frame.size.width / mMetalLayer.frame.size.height);
        float fov = 90 * M_PI / 180; // In radians
        
        // Clipping Planes
        float nearZ = 0.1f;
        float farZ = 1000.f;
        
        matrix_float4x4 perspectiveMat = matrix_perspective_left_hand(fov, aspectRatio, nearZ, farZ);
        TransformData transData = { modelMat, viewMat, perspectiveMat };
        memcpy(mTransformBuffer->contents(), &transData, sizeof transData); // Probably could move
        
        rce->setFrontFacingWinding(MTL::WindingClockwise);
        rce->setCullMode(MTL::CullModeBack);
        rce->setRenderPipelineState(mRenderPipelineState);
        rce->setDepthStencilState(mDepthStencilState);
        rce->setVertexBuffer(mVertexBuffer, 0, 0);
        rce->setVertexBuffer(mTransformBuffer, 0, 1);
        
        auto type = MTL::PrimitiveTypeTriangle;
        NS::UInteger vStart = 0;
        NS::UInteger vCount = 36;
        rce->setFragmentTexture(mTexture->texture, 0);
        rce->drawPrimitives(type, vStart, vCount);
    }
    
    
    // Frame Buffer Callback Section
    void AtomCore::frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
        AtomCore* woah = (AtomCore*)glfwGetWindowUserPointer(window);
        
        woah->resizeFrameBuffer(width, height);
    }
    
    void AtomCore::resizeFrameBuffer(int width, int height) {
        mMetalLayer.drawableSize = CGSizeMake(width, height);
        
        if (mMSAARenderTargetTexture) {
            mMSAARenderTargetTexture->release();
            mMSAARenderTargetTexture = nullptr;
        }
        
        if (mDepthTexture) {
            mDepthTexture->release();
            mDepthTexture = nullptr;
        }
        
        createDepthAndMSAATextures();
        mMetalDrawable = (__bridge CA::MetalDrawable*)[mMetalLayer nextDrawable];
        updateRenderPassDescriptor();
    }
    
}
