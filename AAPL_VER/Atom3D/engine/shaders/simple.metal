//
//  simple.metal
//  Atom3D
//
//  Created by Jens Eckert on 10/4/23.
//

#include <metal_stdlib>
using namespace metal;

#include "../headers/VertexData.hpp"

struct VertexOut {
    float4 position [[position]];
    float2 textureCoords;
};

vertex VertexOut vertexShader(uint vertexId [[vertex_id]],
                              constant Atom::VertexData* vData,
                              constant Atom::TransformData* tData) {
    VertexOut out;
    out.position = tData->perspectiveMatrix * tData->viewMatrix * tData->modelMatrix * vData[vertexId].position;
    out.textureCoords = vData[vertexId].textureCoords;
    return out;
}

fragment float4 fragmentShader(VertexOut in [[stage_in]],
                               texture2d<float> colorTexture [[texture(0)]]) {
    constexpr sampler textureSampler(mag_filter::linear, mag_filter::linear);
    
    const float4 colorSample = colorTexture.sample(textureSampler, in.textureCoords);
    return colorSample;
}
