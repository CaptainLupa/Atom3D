//
//  Texture.hpp
//  Atom3D
//
//  Created by Jens Eckert on 10/5/23.
//

#ifndef Texture_hpp
#define Texture_hpp

#pragma once
#include <Metal/Metal.hpp>
#include "stb_image.h"
#include <iostream>

class Texture {
public:
    Texture(const char*, MTL::Device*);
    ~Texture();
    MTL::Texture* texture;
    int width, height, channels;
    
private:
    MTL::Device* mDevice;
};

#endif /* Texture_hpp */
