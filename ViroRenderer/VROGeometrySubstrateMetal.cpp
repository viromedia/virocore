//
//  VROGeometrySubstrateMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/18/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROGeometrySubstrateMetal.h"
#if VRO_METAL

#include "VROGeometry.h"
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"
#include "VROGeometryUtil.h"
#include "VRODriverMetal.h"
#include "VROMaterialSubstrateMetal.h"
#include "VROMaterial.h"
#include "VROLog.h"
#include "VROSharedStructures.h"
#include "VROMetalUtils.h"
#include "VROConcurrentBuffer.h"
#include <map>

VROGeometrySubstrateMetal::VROGeometrySubstrateMetal(const VROGeometry &geometry,
                                                     VRODriverMetal &driver) {
    id <MTLDevice> device = driver.getDevice();

    readGeometryElements(device, geometry.getGeometryElements());
    readGeometrySources(device, geometry.getGeometrySources());
    updatePipelineStates(geometry, driver);
    
    _viewUniformsBuffer = new VROConcurrentBuffer(sizeof(VROViewUniforms), @"VROViewUniformBuffer", device);
}

VROGeometrySubstrateMetal::~VROGeometrySubstrateMetal() {
    delete (_viewUniformsBuffer);
}

void VROGeometrySubstrateMetal::readGeometryElements(id <MTLDevice> device,
                                                     const std::vector<std::shared_ptr<VROGeometryElement>> &elements) {
    
    for (std::shared_ptr<VROGeometryElement> element : elements) {
        VROGeometryElementMetal elementMetal;
        
        int indexCount = VROGeometryUtilGetIndicesCount(element->getPrimitiveCount(),
                                                        element->getPrimitiveType());
        
        elementMetal.buffer = [device newBufferWithBytes:element->getData()->getData()
                                                  length:indexCount * element->getBytesPerIndex()
                                                 options:0];
        elementMetal.primitiveType = parsePrimitiveType(element->getPrimitiveType());
        elementMetal.indexCount = indexCount;
        elementMetal.indexType = (element->getBytesPerIndex() == 2) ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32;
        elementMetal.indexBufferOffset = 0;
        
        _elements.push_back(elementMetal);
    }
}

void VROGeometrySubstrateMetal::readGeometrySources(id <MTLDevice> device,
                                                    const std::vector<std::shared_ptr<VROGeometrySource>> &sources) {
        
    std::shared_ptr<VROGeometrySource> source = sources.front();
    std::map<std::shared_ptr<VROData>, std::vector<std::shared_ptr<VROGeometrySource>>> dataMap;
    
    /*
     Sort the sources into groups defined by the data array they're using.
     */
    for (std::shared_ptr<VROGeometrySource> source : sources) {
        std::shared_ptr<VROData> data = source->getData();
        
        auto it = dataMap.find(data);
        if (it == dataMap.end()) {
            std::vector<std::shared_ptr<VROGeometrySource>> group = { source };
            dataMap[data] = group;
        }
        else {
            std::vector<std::shared_ptr<VROGeometrySource>> &group = it->second;
            group.push_back(source);
        }
    }
    
    _vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
    int bufferIndex = 0;
    
    /*
     Our shaders currently only support a single data array, so we abort if we have multiple
     arrays (i.e. if we have geometry sources that aren't interleaved into a single array).
     */
    passert (dataMap.size() == 1);
    auto iterator = dataMap.begin();
    
    std::vector<std::shared_ptr<VROGeometrySource>> group = iterator->second;
    
    /*
     Create an MTLBuffer that wraps over the VROData.
     */
    int dataSize = 0;
    for (std::shared_ptr<VROGeometrySource> source : group) {
        int size = source->getVertexCount() * source->getDataStride();
        dataSize = std::max(dataSize, size);
    }
    
    _var.buffer = [device newBufferWithBytes:iterator->first->getData()
                                     length:dataSize options:0];
    _var.buffer.label = @"VROGeometryVertexBuffer";
    
    /*
     Create the layout for this MTL buffer.
     */
    _vertexDescriptor.layouts[bufferIndex].stepRate = 1;
    _vertexDescriptor.layouts[bufferIndex].stride = group[0]->getDataStride();
    _vertexDescriptor.layouts[bufferIndex].stepFunction = MTLVertexStepFunctionPerVertex;
    
    /*
     Create an attribute for each geometry source in this group.
     */
    for (int i = 0; i < group.size(); i++) {
        std::shared_ptr<VROGeometrySource> source = group[i];
        int attrIdx = VROGeometryUtilParseAttributeIndex(source->getSemantic());
        
        _vertexDescriptor.attributes[attrIdx].format = parseVertexFormat(source);
        _vertexDescriptor.attributes[attrIdx].offset = source->getDataOffset();
        _vertexDescriptor.attributes[attrIdx].bufferIndex = bufferIndex;
        
        passert (source->getDataStride() == _vertexDescriptor.layouts[bufferIndex].stride);
    }
}

void VROGeometrySubstrateMetal::updatePipelineStates(const VROGeometry &geometry,
                                                     VRODriverMetal &driver) {
    
    id <MTLDevice> device = driver.getDevice();
    const std::vector<std::shared_ptr<VROMaterial>> &materials = geometry.getMaterials();
    
    for (int i = 0; i < _elements.size(); i++) {
        VROGeometryElementMetal element = _elements[i];
        const std::shared_ptr<VROMaterial> &material = materials[i % materials.size()];
        
        id <MTLRenderPipelineState> pipelineState = createRenderPipelineState(material, driver);
        _elementPipelineStates.push_back(pipelineState);
        
        id <MTLDepthStencilState> depthStencilState = createDepthStencilState(material, device);
        _elementDepthStates.push_back(depthStencilState);
    }
}

id <MTLRenderPipelineState> VROGeometrySubstrateMetal::createRenderPipelineState(const std::shared_ptr<VROMaterial> &material,
                                                                                 VRODriverMetal &driver) {
    
    id <MTLDevice> device = driver.getDevice();
    std::shared_ptr<VRORenderTarget> renderTarget = driver.getRenderTarget();
    
    VROMaterialSubstrateMetal *substrate = static_cast<VROMaterialSubstrateMetal *>(material->getSubstrate(driver));
    
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"VROLayerPipeline";
    pipelineStateDescriptor.sampleCount = renderTarget->getSampleCount();
    pipelineStateDescriptor.vertexFunction = substrate->getVertexProgram();
    pipelineStateDescriptor.fragmentFunction = substrate->getFragmentProgram();
    pipelineStateDescriptor.vertexDescriptor = _vertexDescriptor;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = renderTarget->getColorPixelFormat();
    pipelineStateDescriptor.colorAttachments[0].blendingEnabled = YES;
    pipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    pipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    pipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    pipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    pipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    pipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    pipelineStateDescriptor.depthAttachmentPixelFormat = renderTarget->getDepthStencilPixelFormat();
    pipelineStateDescriptor.stencilAttachmentPixelFormat = renderTarget->getDepthStencilPixelFormat();
    
    NSError *error = NULL;
    id <MTLRenderPipelineState> pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor
                                                                                       error:&error];
    if (!pipelineState) {
        NSLog(@"Failed to created pipeline state, error %@", error);
    }
    return  pipelineState;
}

id <MTLDepthStencilState> VROGeometrySubstrateMetal::createDepthStencilState(const std::shared_ptr<VROMaterial> &material,
                                                                             id <MTLDevice> device) {
    
    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthWriteEnabled = material->getWritesToDepthBuffer();
    
    if (material->getReadsFromDepthBuffer()) {
        /*
         Using LessEqual ensures that outgoing material transitions work correctly,
         in that we can render the same face twice (once outgoing, once incoming), and
         the incoming will not fail the depth test despite having the same depth as
         the outgoing.
         */
        depthStateDesc.depthCompareFunction = MTLCompareFunctionLessEqual;
    }
    else {
        depthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
    }
    
    return [device newDepthStencilStateWithDescriptor:depthStateDesc];
}

MTLVertexFormat VROGeometrySubstrateMetal::parseVertexFormat(std::shared_ptr<VROGeometrySource> &source) {
    // Currently assuming floats
    switch (source->getBytesPerComponent()) {
        case 2:
            switch (source->getComponentsPerVertex()) {
                case 1:
                    return MTLVertexFormatFloat;
                    
                case 2:
                    return MTLVertexFormatFloat2;
                    
                case 3:
                    return MTLVertexFormatFloat3;
                    
                case 4:
                    return MTLVertexFormatFloat4;
                    
                default:
                    pabort();
                    return MTLVertexFormatFloat;
            }
            
        case 4:
            switch (source->getComponentsPerVertex()) {
                case 1:
                    return MTLVertexFormatFloat;
                    
                case 2:
                    return MTLVertexFormatFloat2;
                    
                case 3:
                    return MTLVertexFormatFloat3;
                    
                case 4:
                    return MTLVertexFormatFloat4;
                    
                default:
                    pabort();
                    return MTLVertexFormatFloat;
            }
            
        default:
            pabort();
            return MTLVertexFormatFloat;
    }
}

MTLPrimitiveType VROGeometrySubstrateMetal::parsePrimitiveType(VROGeometryPrimitiveType primitive) {
    switch (primitive) {
        case VROGeometryPrimitiveType::Triangle:
            return MTLPrimitiveTypeTriangle;
            
        case VROGeometryPrimitiveType::TriangleStrip:
            return MTLPrimitiveTypeTriangleStrip;
            
        case VROGeometryPrimitiveType::Line:
            return MTLPrimitiveTypeLine;
            
        case VROGeometryPrimitiveType::Point:
            return MTLPrimitiveTypePoint;
            
        default:
            break;
    }
}

void VROGeometrySubstrateMetal::render(const VROGeometry &geometry,
                                       int elementIndex,
                                       VROMatrix4f transform,
                                       VROMatrix4f normalMatrix,
                                       float opacity,
                                       std::shared_ptr<VROMaterial> &material,
                                       const VRORenderContext &context,
                                       std::shared_ptr<VRODriver> &driver) {
    
    VRODriverMetal &metal = (VRODriverMetal &)driver;
    id <MTLRenderCommandEncoder> renderEncoder = metal.getRenderTarget()->getRenderEncoder();
    
    int frame = context.getFrame();
    VROEyeType eyeType = context.getEyeType();
    
    VROMatrix4f viewMatrix = context.getViewMatrix();
    VROMatrix4f projectionMatrix = context.getProjectionMatrix();
    
    if (geometry.isCameraEnclosure()) {
        viewMatrix = context.getEnclosureViewMatrix();
    }
    
    [renderEncoder pushDebugGroup:@"VROGeometry"];
    VROGeometryElementMetal element = _elements[elementIndex];
    
    /*
     Configure the view uniforms.
     */
    VROMatrix4f modelview = viewMatrix.multiply(transform);
    VROViewUniforms *viewUniforms = (VROViewUniforms *)_viewUniformsBuffer->getWritableContents(eyeType, frame);
    
    viewUniforms->normal_matrix = toMatrixFloat4x4(normalMatrix);
    viewUniforms->model_matrix = toMatrixFloat4x4(transform);
    viewUniforms->modelview_matrix = toMatrixFloat4x4(modelview);
    viewUniforms->modelview_projection_matrix = toMatrixFloat4x4(projectionMatrix.multiply(modelview));
    viewUniforms->camera_position = toVectorFloat3(context.getCamera().getPosition());
    
    [renderEncoder setVertexBuffer:_viewUniformsBuffer->getMTLBuffer(eyeType)
                            offset:_viewUniformsBuffer->getWriteOffset(frame) atIndex:1];
    
    /*
     Determine if the material has been updated. If so, we need to update our pipeline and
     depth states.
     */
    if (material->isUpdated()) {
        _elementPipelineStates[elementIndex] = createRenderPipelineState(material, metal);
        _elementDepthStates[elementIndex] = createDepthStencilState(material, metal.getDevice());
    }
    
    VROMaterialSubstrateMetal *substrate = static_cast<VROMaterialSubstrateMetal *>(material->getSubstrate(driver));
    id <MTLRenderPipelineState> pipelineState = _elementPipelineStates[elementIndex];
    id <MTLDepthStencilState> depthState = _elementDepthStates[elementIndex];
    
    [renderEncoder setVertexBuffer:_var.buffer offset:0 atIndex:0];
    
    /*
     Note that outgoing materials share the same pipeline state as their counterparts. This is because
     they always have the same shaders and vertex layouts.
     */
    renderMaterial(substrate, element, pipelineState, depthState, renderEncoder, opacity,
                   context, driver);
    
    [renderEncoder popDebugGroup];
}

void VROGeometrySubstrateMetal::renderMaterial(VROMaterialSubstrateMetal *material,
                                               VROGeometryElementMetal &element,
                                               id <MTLRenderPipelineState> pipelineState,
                                               id <MTLDepthStencilState> depthStencilState,
                                               id <MTLRenderCommandEncoder> renderEncoder,
                                               float opacity,
                                               const VRORenderContext &renderContext,
                                               std::shared_ptr<VRODriver> &driver) {
    
    int frame = renderContext.getFrame();
    VROEyeType eyeType = renderContext.getEyeType();
    
    [renderEncoder setRenderPipelineState:pipelineState];
    [renderEncoder setDepthStencilState:depthStencilState];
    
    VROConcurrentBuffer &materialBuffer = material->bindMaterialUniforms(opacity, eyeType, frame);
    [renderEncoder setVertexBuffer:materialBuffer.getMTLBuffer(eyeType)
                            offset:materialBuffer.getWriteOffset(frame)
                           atIndex:2];
    
    const std::vector<std::shared_ptr<VROTexture>> &textures = material->getTextures();
    for (int j = 0; j < textures.size(); ++j) {
        VROTextureSubstrateMetal *substrate = (VROTextureSubstrateMetal *) textures[j]->getSubstrate(driver);
        if (!substrate) {
            // Use a blank placeholder if a texture is not yet available (i.e.
            // during video texture loading)
            std::shared_ptr<VROTexture> blank = getBlankTexture();
            substrate = (VROTextureSubstrateMetal *) blank->getSubstrate(driver);
        }
        
        [renderEncoder setFragmentTexture:substrate->getTexture() atIndex:j];
    }
    
    [renderEncoder drawIndexedPrimitives:element.primitiveType
                              indexCount:element.indexCount
                               indexType:element.indexType
                             indexBuffer:element.buffer
                       indexBufferOffset:0];
}

#endif
