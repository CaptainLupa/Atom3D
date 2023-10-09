//
//  VertexData.hpp
//  Atom3D
//
//  Created by Jens Eckert on 10/5/23.
//

#ifndef VertexData_h
#define VertexData_h

#include <simd/simd.h>

using namespace simd;

namespace Atom {

struct VertexData {
    float4 position;
    float2 textureCoords;
};

struct TransformData {
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    float4x4 perspectiveMatrix;
};

}

#endif /* VertexData_h */
