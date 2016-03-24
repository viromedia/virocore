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
#include "VROMaterial.h"
#include "VROLog.h"
#include "VROSharedStructures.h"
#include "VROMetalUtils.h"
#include "VRORenderParameters.h"
#include "VROConcurrentBuffer.h"
#include <map>

VROGeometrySubstrateMetal::VROGeometrySubstrateMetal(const VROGeometry &geometry,
                                                     const VRORenderContextMetal &context) {
    id <MTLDevice> device = context.getDevice();

    readGeometryElements(device, geometry.getGeometryElements());
    readGeometrySources(device, geometry.getGeometrySources());
    updatePipelineStates(geometry, context);
    
    for (int i = 0; i < _elements.size(); i++) {
        _outgoingPipelineStates.push_back(nullptr);
    }
    
    _viewUniformsBuffer = new VROConcurrentBuffer(sizeof(VROViewUniforms), @"VROViewUniformBuffer", device);
}

VROGeometrySubstrateMetal::~VROGeometrySubstrateMetal() {
    delete (_viewUniformsBuffer);
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

void VROGeometrySubstrateMetal::updatePipelineStates(const VROGeometry &geometry,
                                                     const VRORenderContextMetal &context) {
    
    id <MTLDevice> device = context.getDevice();
    const std::vector<std::shared_ptr<VROMaterial>> &materials = geometry.getMaterials_const();
    
    for (int i = 0; i < _elements.size(); i++) {
        VROGeometryElementMetal element = _elements[i];
        const std::shared_ptr<VROMaterial> &material = materials[i % materials.size()];
        
        id <MTLRenderPipelineState> pipelineState = createRenderPipelineState(material, context);
        _elementPipelineStates.push_back(pipelineState);
        
        id <MTLDepthStencilState> depthStencilState = createDepthStencilState(material, device);
        _elementDepthStates.push_back(depthStencilState);
    }
}

id <MTLRenderPipelineState> VROGeometrySubstrateMetal::createRenderPipelineState(const std::shared_ptr<VROMaterial> &material,
                                                                                 const VRORenderContextMetal &context) {
    
    id <MTLDevice> device = context.getDevice();
    std::shared_ptr<VRORenderTarget> renderTarget = context.getRenderTarget();
    
    material->createSubstrate(context);
    VROMaterialSubstrateMetal *substrate = static_cast<VROMaterialSubstrateMetal *>(material->getSubstrate());
    
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

void VROGeometrySubstrateMetal::render(const VROGeometry &geometry,
                                       const std::vector<std::shared_ptr<VROMaterial>> &materials,
                                       const VRORenderContext &context,
                                       VRORenderParameters &params) {
    
    const VRORenderContextMetal &metal = (VRORenderContextMetal &)context;
    id <MTLRenderCommandEncoder> renderEncoder = metal.getRenderTarget()->getRenderEncoder();
    
    int frame = context.getFrame();
    VROEyeType eyeType = context.getEyeType();
    VROMatrix4f &transform = params.transforms.top();
    
    VROMatrix4f viewMatrix = context.getViewMatrix();
    VROMatrix4f projectionMatrix = context.getProjectionMatrix();
    
    if (!geometry.isStereoRenderingEnabled()) {
        viewMatrix = context.getMonocularViewMatrix();
    }
    
    for (int i = 0; i < _elements.size(); i++) {
        [renderEncoder pushDebugGroup:@"VROGeometry"];
        VROGeometryElementMetal element = _elements[i];
        
        /*
         Configure the view uniforms.
         */
        VROMatrix4f modelview = viewMatrix.multiply(transform);
        VROViewUniforms *viewUniforms = (VROViewUniforms *)_viewUniformsBuffer->getWritableContents(eyeType, frame);

        viewUniforms->normal_matrix = toMatrixFloat4x4(transform.invert().transpose());
        viewUniforms->model_matrix = toMatrixFloat4x4(transform);
        viewUniforms->modelview_matrix = toMatrixFloat4x4(modelview);
        viewUniforms->modelview_projection_matrix = toMatrixFloat4x4(projectionMatrix.multiply(modelview));
        viewUniforms->camera_position = toVectorFloat3(context.getCamera().getPosition());

        [renderEncoder setVertexBuffer:_viewUniformsBuffer->getMTLBuffer(eyeType)
                                offset:_viewUniformsBuffer->getWriteOffset(frame) atIndex:_vars.size()];

        /*
         Determine if the material has been updated. If so, we need to update our pipeline and
         depth states.
         */
        const std::shared_ptr<VROMaterial> &material = materials[i % materials.size()];
        if (material->isUpdated()) {
            _elementPipelineStates[i] = createRenderPipelineState(material, metal);
            _elementDepthStates[i] = createDepthStencilState(material, metal.getDevice());
        }
        
        VROMaterialSubstrateMetal *substrate = static_cast<VROMaterialSubstrateMetal *>(material->getSubstrate());
        id <MTLRenderPipelineState> pipelineState = _elementPipelineStates[i];
        id <MTLDepthStencilState> depthState = _elementDepthStates[i];
        
        for (int j = 0; j < _vars.size(); ++j) {
            [renderEncoder setVertexBuffer:_vars[j].buffer offset:0 atIndex:j];
        }
        
        VROConcurrentBuffer &lightingBuffer = substrate->bindLightingUniforms(params.lights, eyeType, frame);
    
        [renderEncoder setVertexBuffer:lightingBuffer.getMTLBuffer(eyeType)
                                offset:lightingBuffer.getWriteOffset(frame)
                               atIndex:_vars.size() + 2];
        [renderEncoder setFragmentBuffer:lightingBuffer.getMTLBuffer(eyeType)
                                  offset:lightingBuffer.getWriteOffset(frame)
                                 atIndex:0];

        const std::shared_ptr<VROMaterial> &outgoing = material->getOutgoing();
        if (outgoing) {
            if (_outgoingPipelineStates[i] == nullptr || outgoing->isUpdated()) {
                _outgoingPipelineStates[i] = createRenderPipelineState(outgoing, metal);
            }
            
            id <MTLRenderPipelineState> outgoingPipelineState = _outgoingPipelineStates[i];
            VROMaterialSubstrateMetal *outgoingSubstrate = static_cast<VROMaterialSubstrateMetal *>(outgoing->getSubstrate());
            
            renderMaterial(outgoingSubstrate, element, outgoingPipelineState, depthState, renderEncoder, params, context);
            renderMaterial(substrate, element, pipelineState, depthState, renderEncoder, params, context);
        }
        else {
            _outgoingPipelineStates[i] = nullptr;
            renderMaterial(substrate, element, pipelineState, depthState, renderEncoder, params, context);
        }
        
        [renderEncoder popDebugGroup];
    }
}

void VROGeometrySubstrateMetal::renderMaterial(VROMaterialSubstrateMetal *material,
                                               VROGeometryElementMetal &element,
                                               id <MTLRenderPipelineState> pipelineState,
                                               id <MTLDepthStencilState> depthStencilState,
                                               id <MTLRenderCommandEncoder> renderEncoder,
                                               VRORenderParameters &params,
                                               const VRORenderContext &context) {
    
    int frame = context.getFrame();
    VROEyeType eyeType = context.getEyeType();
    
    [renderEncoder setRenderPipelineState:pipelineState];
    [renderEncoder setDepthStencilState:depthStencilState];
    
    VROConcurrentBuffer &materialBuffer = material->bindMaterialUniforms(params, eyeType, frame);
    [renderEncoder setVertexBuffer:materialBuffer.getMTLBuffer(eyeType)
                            offset:materialBuffer.getWriteOffset(frame)
                           atIndex:_vars.size() + 1];
    
    const std::vector<std::shared_ptr<VROTexture>> &textures = material->getTextures();
    for (int j = 0; j < textures.size(); ++j) {
        VROTextureSubstrateMetal *substrate = (VROTextureSubstrateMetal *) textures[j]->getSubstrate(context);
        if (!substrate) {
            // Use a blank placeholder if a texture is not yet available (i.e.
            // during video texture loading)
            std::shared_ptr<VROTexture> blank = getBlankTexture();
            substrate = (VROTextureSubstrateMetal *) blank->getSubstrate(context);
        }
        
        [renderEncoder setFragmentTexture:substrate->getTexture() atIndex:j];
    }
    
    [renderEncoder drawIndexedPrimitives:element.primitiveType
                              indexCount:element.indexCount
                               indexType:element.indexType
                             indexBuffer:element.buffer
                       indexBufferOffset:0];
}
