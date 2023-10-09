//
//  Texture.cpp
//  Atom3D
//
//  Created by Jens Eckert on 10/5/23.
//

#include "Texture.hpp"

namespace Atom {

Texture::Texture(const char* filePath, MTL::Device* devicePtr) {
    mDevice = devicePtr;
    
    stbi_set_flip_vertically_on_load(true);
    
    unsigned char* image = stbi_load(filePath, &width, &height, &channels, STBI_rgb_alpha);
    if (image == nullptr) {
        std::cout << "Could not load image at " << filePath << "\n";
        std::exit(-1);
    }
    
    MTL::TextureDescriptor* textureDesc = MTL::TextureDescriptor::alloc()->init();
    textureDesc->setPixelFormat(MTL::PixelFormatRGBA8Unorm);
    textureDesc->setWidth(width);
    textureDesc->setHeight(height);
    
    texture = mDevice->newTexture(textureDesc);
    
    MTL::Region region = MTL::Region(0, 0, 0, width, height, 1);
    NS::UInteger bytesPerRow = 4 * width;
    
    texture->replaceRegion(region, 0, image, bytesPerRow);
    
    textureDesc->release();
    stbi_image_free(image);
}

Texture::~Texture() {
    texture->release();
}

}
