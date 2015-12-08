//
//  VROGeometrySubstrateMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/18/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROGeometrySubstrateMetal.h"
#include "VROGeometry.h"
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"
#include "VRORenderContextMetal.h"
#include "VROMaterialSubstrateMetal.h"
#include "VROLog.h"
#include "SharedStructures.h"
#include "VROMetalUtils.h"
#include <map>

VROGeometrySubstrateMetal::VROGeometrySubstrateMetal(const VROGeometry &geometry,
                                                     const VRORenderContextMetal &context) {
    id <MTLDevice> device = context.getDevice();

    readGeometryElements(device, geometry.getGeometryElements());
    readGeometrySources(device, geometry.getGeometrySources());
    
    for (const std::shared_ptr<VROMaterial> &material : geometry.getMaterials_const()) {
        _materials.push_back(new VROMaterialSubstrateMetal(*material.get(), context));
    }
    
    for (int i = 0; i < _elements.size(); i++) {
        VROGeometryElementMetal element = _elements[i];
        
        int materialIdx = i % _materials.size();
        VROMaterialSubstrateMetal *material = _materials[materialIdx];
        
        id <MTLFunction> vertexProgram = material->getVertexProgram();
        id <MTLFunction> fragmentProgram = material->getFragmentProgram();
        
        const VRORenderContextMetal &metal = (VRORenderContextMetal &)context;
        
        std::shared_ptr<VRORenderTarget> renderTarget = metal.getRenderTarget();
        
        MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineStateDescriptor.label = @"VROLayerPipeline";
        pipelineStateDescriptor.sampleCount = renderTarget->getSampleCount();
        pipelineStateDescriptor.vertexFunction = vertexProgram;
        pipelineStateDescriptor.fragmentFunction = fragmentProgram;
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
        
        _elementPipelineStates.push_back(pipelineState);
        
        MTLDepthStencilDescriptor *depthStateDesc = parseDepthStencil(geometry.getMaterials_const()[materialIdx]);
        _elementDepthStates.push_back([device newDepthStencilStateWithDescriptor:depthStateDesc]);
    }
    
    _viewUniformsBuffer = [device newBufferWithLength:sizeof(VROViewUniforms) options:0];
    _viewUniformsBuffer.label = @"VROViewUniformBuffer";
}

VROGeometrySubstrateMetal::~VROGeometrySubstrateMetal() {
    for (VROMaterialSubstrateMetal *material : _materials) {
        delete (material);
    }
}

void VROGeometrySubstrateMetal::readGeometryElements(id <MTLDevice> device,
                                                     const std::vector<std::shared_ptr<VROGeometryElement>> &elements) {
    
    for (std::shared_ptr<VROGeometryElement> element : elements) {
        VROGeometryElementMetal elementMetal;
        
        int indexCount = getIndicesCount(element->getPrimitiveCount(), element->getPrimitiveType());
        
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
     For each group of GeometrySources we create an MTLBuffer and layout.
     */
    for (auto &kv : dataMap) {
        VROVertexArrayMetal var;
        std::vector<std::shared_ptr<VROGeometrySource>> group = kv.second;
        
        /*
         Create an MTLBuffer that wraps over the VROData.
         */
        int dataSize = 0;
        for (std::shared_ptr<VROGeometrySource> source : group) {
            int size = source->getVertexCount() * source->getDataStride();
            dataSize = std::max(dataSize, size);
        }
        
        var.buffer = [device newBufferWithBytes:kv.first->getData()
                                         length:dataSize options:0];
        var.buffer.label = @"VROGeometryVertexBuffer";
        
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
            int attrIdx = parseAttributeIndex(source->getSemantic());
            
            _vertexDescriptor.attributes[attrIdx].format = parseVertexFormat(source);
            _vertexDescriptor.attributes[attrIdx].offset = source->getDataOffset();
            _vertexDescriptor.attributes[attrIdx].bufferIndex = bufferIndex;
            
            passert (source->getDataStride() == _vertexDescriptor.layouts[bufferIndex].stride);
        }
        
        _vars.push_back(var);
        ++bufferIndex;
    }
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

int VROGeometrySubstrateMetal::getIndicesCount(int primitiveCount, VROGeometryPrimitiveType primitiveType) {
    switch (primitiveType) {
        case VROGeometryPrimitiveType::Triangle:
            return primitiveCount * 3;
            
        case VROGeometryPrimitiveType::TriangleStrip:
            return primitiveCount + 2;
            
        case VROGeometryPrimitiveType::Line:
            return primitiveCount * 2;
            
        case VROGeometryPrimitiveType::Point:
            return primitiveCount;
            
        default:
            break;
    }
}

int VROGeometrySubstrateMetal::parseAttributeIndex(VROGeometrySourceSemantic semantic) {
    switch (semantic) {
        case VROGeometrySourceSemantic::Vertex:
            return 0;
        case VROGeometrySourceSemantic::Normal:
            return 1;
        case VROGeometrySourceSemantic::Color:
            return 2;
        case VROGeometrySourceSemantic::Texcoord:
            return 3;
        case VROGeometrySourceSemantic::VertexCrease:
            return 4;
        case VROGeometrySourceSemantic::EdgeCrease:
            return 5;
        case VROGeometrySourceSemantic::BoneWeights:
            return 6;
        case VROGeometrySourceSemantic::BoneIndices:
            return 7;
        default:
            return 0;
    }
}

MTLDepthStencilDescriptor *VROGeometrySubstrateMetal::parseDepthStencil(const std::shared_ptr<VROMaterial> &material) {
    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthWriteEnabled = material->getWritesToDepthBuffer();
    
    if (material->getReadsFromDepthBuffer()) {
        depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
    }
    else {
        depthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
    }
    
    return depthStateDesc;
}


void VROGeometrySubstrateMetal::render(const VRORenderContext &context,
                                       const VROMatrix4f &transform,
                                       const std::vector<std::shared_ptr<VROLight>> &lights) {
    
    const VRORenderContextMetal &metal = (VRORenderContextMetal &)context;

    for (const std::shared_ptr<VROLight> &light : lights) {
        
        for (int i = 0; i < _elements.size(); i++) {
            VROGeometryElementMetal element = _elements[i];
            
            VROMaterialSubstrateMetal *material = _materials[i % _materials.size()];
            material->setLightingUniforms(light);
            
            id <MTLRenderPipelineState> pipelineState = _elementPipelineStates[i];
            id <MTLDepthStencilState> depthState = _elementDepthStates[i];
            
            /*
             Configure the view uniforms.
             */
            VROViewUniforms *viewUniforms = (VROViewUniforms *)[_viewUniformsBuffer contents];
            viewUniforms->normal_matrix = toMatrixFloat4x4(transform.transpose().invert());
            viewUniforms->modelview_projection_matrix = toMatrixFloat4x4(metal.getProjectionMatrix().multiply(transform));
            
            id <MTLRenderCommandEncoder> renderEncoder = metal.getRenderTarget()->getRenderEncoder();
            [renderEncoder pushDebugGroup:@"VROGeometry"];
            [renderEncoder setDepthStencilState:depthState];
            [renderEncoder setRenderPipelineState:pipelineState];
            
            for (int j = 0; j < _vars.size(); ++j) {
                [renderEncoder setVertexBuffer:_vars[j].buffer offset:0 atIndex:j];
            }
            [renderEncoder setVertexBuffer:_viewUniformsBuffer offset:0 atIndex:_vars.size()];
            [renderEncoder setVertexBuffer:material->getLightingUniformsBuffer() offset:0 atIndex:_vars.size() + 1];
            
            const std::vector<id <MTLTexture>> &textures = material->getTextures();
            for (int j = 0; j < textures.size(); ++j) {
                [renderEncoder setFragmentTexture:textures[j] atIndex:j];
            }
            
            [renderEncoder drawIndexedPrimitives:element.primitiveType
                                      indexCount:element.indexCount
                                       indexType:element.indexType
                                     indexBuffer:element.buffer
                               indexBufferOffset:0];
            [renderEncoder popDebugGroup];
        }
    }
}
