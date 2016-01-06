//
//  VRODistortionMesh.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/29/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VRODistortionMesh.h"
#include "VROMath.h"

VRODistortionMesh::VRODistortionMesh(const VRODistortion distortionRed,
                                     const VRODistortion distortionGreen,
                                     const VRODistortion distortionBlue,
                                     float screenWidth, float screenHeight,
                                     float xEyeOffsetScreen, float yEyeOffsetScreen,
                                     float textureWidth, float textureHeight,
                                     float xEyeOffsetTexture, float yEyeOffsetTexture,
                                     float viewportXTexture, float viewportYTexture,
                                     float viewportWidthTexture, float viewportHeightTexture,
                                     bool vignetteEnabled,
                                     id <MTLDevice> gpu) {
    float vertexData[14400];
    int vertexOffset = 0;
    
    const int rows = 40;
    const int cols = 40;
    
    const float vignetteSizeTanAngle = 0.05f;
    
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            const float uTextureBlue = col / 39.0f * (viewportWidthTexture / textureWidth) + viewportXTexture / textureWidth;
            const float vTextureBlue = row / 39.0f * (viewportHeightTexture / textureHeight) + viewportYTexture / textureHeight;
            
            const float xTexture = uTextureBlue * textureWidth - xEyeOffsetTexture;
            const float yTexture = vTextureBlue * textureHeight - yEyeOffsetTexture;
            const float rTexture = sqrtf(xTexture * xTexture + yTexture * yTexture);
            
            const float textureToScreenBlue = (rTexture > 0.0f) ? distortionBlue.distortInverse(rTexture) / rTexture : 1.0f;
            
            const float xScreen = xTexture * textureToScreenBlue;
            const float yScreen = yTexture * textureToScreenBlue;
            
            const float uScreen = (xScreen + xEyeOffsetScreen) / screenWidth;
            const float vScreen = (yScreen + yEyeOffsetScreen) / screenHeight;
            const float rScreen = rTexture * textureToScreenBlue;
            
            const float screenToTextureGreen = (rScreen > 0.0f) ? distortionGreen.getDistortionFactor(rScreen) : 1.0f;
            const float uTextureGreen = (xScreen * screenToTextureGreen + xEyeOffsetTexture) / textureWidth;
            const float vTextureGreen = (yScreen * screenToTextureGreen + yEyeOffsetTexture) / textureHeight;
            
            const float screenToTextureRed = (rScreen > 0.0f) ? distortionRed.getDistortionFactor(rScreen) : 1.0f;
            const float uTextureRed = (xScreen * screenToTextureRed + xEyeOffsetTexture) / textureWidth;
            const float vTextureRed = (yScreen * screenToTextureRed + yEyeOffsetTexture) / textureHeight;
            
            const float vignetteSizeTexture = vignetteSizeTanAngle / textureToScreenBlue;
            
            const float dxTexture = xTexture + xEyeOffsetTexture - clamp(xTexture + xEyeOffsetTexture,
                                                                         viewportXTexture + vignetteSizeTexture,
                                                                         viewportXTexture + viewportWidthTexture - vignetteSizeTexture);
            const float dyTexture = yTexture + yEyeOffsetTexture - clamp(yTexture + yEyeOffsetTexture,
                                                                         viewportYTexture + vignetteSizeTexture,
                                                                         viewportYTexture + viewportHeightTexture - vignetteSizeTexture);
            const float drTexture = sqrtf(dxTexture * dxTexture + dyTexture * dyTexture);
            
            float vignette = 1.0f;
            if (vignetteEnabled) {
                vignette = 1.0f - clamp(drTexture / vignetteSizeTexture, 0.0f, 1.0f);
            }
            
            vertexData[(vertexOffset + 0)] = 2.0f * uScreen - 1.0f;
            vertexData[(vertexOffset + 1)] = 2.0f * vScreen - 1.0f;
            vertexData[(vertexOffset + 2)] = vignette;
            vertexData[(vertexOffset + 3)] = uTextureRed;
            
            // TODO For Metal, we invert the V texture coordinate (top-left origin in Metal)
            //      Fix this for OpenGL by removing the (1.0 - v) for each.
            vertexData[(vertexOffset + 4)] = 1.0 - vTextureRed;
            vertexData[(vertexOffset + 5)] = uTextureGreen;
            vertexData[(vertexOffset + 6)] = 1.0 - vTextureGreen;
            vertexData[(vertexOffset + 7)] = uTextureBlue;
            vertexData[(vertexOffset + 8)] = 1.0 - vTextureBlue;
            
            vertexOffset += 9;
        }
    }
    
    int numIndices = 3158;
    uint16_t indexData[numIndices];
    
    int indexOffset = 0;
    vertexOffset = 0;
    for (int row = 0; row < rows-1; row++) {
        if (row > 0) {
            indexData[indexOffset] = indexData[(indexOffset - 1)];
            indexOffset++;
        }
        for (int col = 0; col < cols; col++) {
            if (col > 0) {
                if (row % 2 == 0) {
                    vertexOffset++;
                }
                else {
                    vertexOffset--;
                }
            }
            indexData[(indexOffset++)] = vertexOffset;
            indexData[(indexOffset++)] = (vertexOffset + 40);
        }
        vertexOffset += 40;
    }
    
    _vertexBuffer = [gpu newBufferWithLength:sizeof(vertexData) options:0];
    _vertexBuffer.label = @"VRODistortionMeshVertexBuffer";
    
    // TODO Switch to using newBufferWithBytesNoCopy?
    memcpy([_vertexBuffer contents], vertexData, sizeof(vertexData));
    
    _indexBuffer = [gpu newBufferWithLength:sizeof(indexData) options:0];
    _indexBuffer.label = @"VRODistortionMeshIndexBuffer";
    
    memcpy([_indexBuffer contents], indexData, sizeof(indexData));
}

void VRODistortionMesh::render(id <MTLRenderCommandEncoder> renderEncoder) const {
    [renderEncoder setVertexBuffer:_vertexBuffer offset:0 atIndex:0];
    [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangleStrip
                              indexCount:[_indexBuffer length] / sizeof(short)
                               indexType:MTLIndexTypeUInt16
                             indexBuffer:_indexBuffer
                       indexBufferOffset:0];
}