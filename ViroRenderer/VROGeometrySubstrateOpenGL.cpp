//
//  VROGeometrySubstrateOpenGL.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROGeometrySubstrateOpenGL.h"
#include "VRODriverOpenGL.h"
#include "VROGeometry.h"
#include "VROGeometryElement.h"
#include "VROGeometrySource.h"
#include "VROGeometryUtil.h"
#include "VROLog.h"
#include "VROShaderProgram.h"
#include "VRORenderParameters.h"
#include <map>

VROGeometrySubstrateOpenGL::VROGeometrySubstrateOpenGL(const VROGeometry &geometry,
                                                       const VRODriverOpenGL &driver) {
    
    readGeometryElements(geometry.getGeometryElements());
    readGeometrySources(geometry.getGeometrySources());
    
    _program = new VROShaderProgram("lambert", 0);
    
    VROShaderProperty uniformTypes[1];
    uniformTypes[0] = VROShaderProperty::Mat4;
    
    const char *names[1];
    names[0] = "mvp_matrix";
    
    _program->setUniforms(uniformTypes, names, 1);
    _program->hydrate();
}

VROGeometrySubstrateOpenGL::~VROGeometrySubstrateOpenGL() {
    
}

void VROGeometrySubstrateOpenGL::readGeometryElements(const std::vector<std::shared_ptr<VROGeometryElement>> &elements) {
    
    for (std::shared_ptr<VROGeometryElement> element : elements) {
        VROGeometryElementOpenGL elementOGL;
        
        int indexCount = VROGeometryUtilGetIndicesCount(element->getPrimitiveCount(), element->getPrimitiveType());
        
        glGenBuffers(1, &elementOGL.buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementOGL.buffer);
        glBufferData(elementOGL.buffer, indexCount * element->getBytesPerIndex(), element->getData()->getData(), GL_STATIC_DRAW);
        
        elementOGL.primitiveType = parsePrimitiveType(element->getPrimitiveType());
        elementOGL.indexCount = indexCount;
        elementOGL.indexType = (element->getBytesPerIndex() == 2) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
        elementOGL.indexBufferOffset = 0;
        
        _elements.push_back(elementOGL);
    }
}

void VROGeometrySubstrateOpenGL::readGeometrySources(const std::vector<std::shared_ptr<VROGeometrySource>> &sources) {
    
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
    
    int bufferIndex = 0;
    
    /*
     For each group of GeometrySources we create an MTLBuffer and layout.
     */
    for (auto &kv : dataMap) {
        VROVertexArrayOpenGL var;
        std::vector<std::shared_ptr<VROGeometrySource>> group = kv.second;
        
        /*
         Create an MTLBuffer that wraps over the VROData.
         */
        int dataSize = 0;
        for (std::shared_ptr<VROGeometrySource> source : group) {
            int size = source->getVertexCount() * source->getDataStride();
            dataSize = std::max(dataSize, size);
        }
        
        glGenBuffers(1, &var.buffer);
        glBindBuffer(GL_ARRAY_BUFFER, var.buffer);
        glBufferData(var.buffer, dataSize, kv.first->getData(), GL_STATIC_DRAW);
        
        /*
         Create the layout for this MTL buffer.
         */
        _vertexDescriptor[bufferIndex].stride = group[0]->getDataStride();
        
        /*
         Create an attribute for each geometry source in this group.
         */
        for (int i = 0; i < group.size(); i++) {
            std::shared_ptr<VROGeometrySource> source = group[i];
            int attrIdx = VROGeometryUtilParseAttributeIndex(source->getSemantic());
            
            std::pair<GLuint, int> format = parseVertexFormat(source);
            _vertexDescriptor[bufferIndex].attributes[attrIdx].index = attrIdx;
            _vertexDescriptor[bufferIndex].attributes[attrIdx].size = format.second;
            _vertexDescriptor[bufferIndex].attributes[attrIdx].type = format.first;
            _vertexDescriptor[bufferIndex].attributes[attrIdx].offset = source->getDataOffset();
            
            passert (source->getDataStride() == _vertexDescriptor[bufferIndex].stride);
        }
        
        _vars.push_back(var);
        ++bufferIndex;
    }
}

std::pair<GLuint, int> VROGeometrySubstrateOpenGL::parseVertexFormat(std::shared_ptr<VROGeometrySource> &source) {
    // Currently assuming floats
    switch (source->getBytesPerComponent()) {
        case 2:
            switch (source->getComponentsPerVertex()) {
                case 1:
                    return { GL_FLOAT, 1 };
                    
                case 2:
                    return { GL_FLOAT, 2 };
                    
                case 3:
                    return { GL_FLOAT, 3 };
                    
                case 4:
                    return { GL_FLOAT, 4 };
                    
                default:
                    pabort();
                    return { GL_FLOAT, 1 };
            }
            
        case 4:
            switch (source->getComponentsPerVertex()) {
                case 1:
                    return { GL_FLOAT, 1 };
                    
                case 2:
                    return { GL_FLOAT, 2 };
                    
                case 3:
                    return { GL_FLOAT, 3 };
                    
                case 4:
                    return { GL_FLOAT, 4 };
                    
                default:
                    pabort();
                    return { GL_FLOAT, 1 };
            }
            
        default:
            pabort();
            return { GL_FLOAT, 1 };
    }
}

GLuint VROGeometrySubstrateOpenGL::parsePrimitiveType(VROGeometryPrimitiveType primitive) {
    switch (primitive) {
        case VROGeometryPrimitiveType::Triangle:
            return GL_TRIANGLES;
            
        case VROGeometryPrimitiveType::TriangleStrip:
            return GL_TRIANGLE_STRIP;
            
        case VROGeometryPrimitiveType::Line:
            return GL_LINES;
            
        case VROGeometryPrimitiveType::Point:
            return GL_POINTS;
            
        default:
            break;
    }
}

void VROGeometrySubstrateOpenGL::render(const VROGeometry &geometry,
                                        const std::vector<std::shared_ptr<VROMaterial>> &materials,
                                        const VRORenderContext &renderContext,
                                        const VRODriver &driver,
                                        VRORenderParameters &params) {
    
    int frame = renderContext.getFrame();
    VROEyeType eyeType = renderContext.getEyeType();
    VROMatrix4f &transform = params.transforms.top();
    
    VROMatrix4f viewMatrix = renderContext.getViewMatrix();
    VROMatrix4f projectionMatrix = renderContext.getProjectionMatrix();
    
    if (!geometry.isStereoRenderingEnabled()) {
        viewMatrix = renderContext.getMonocularViewMatrix();
    }
    
    for (int i = 0; i < _elements.size(); i++) {
        pglpush("VROGeometry");
        VROGeometryElementOpenGL element = _elements[i];
        
        /*
         Configure the view uniforms.
         */
        VROMatrix4f modelview = viewMatrix.multiply(transform);
        
        /*
        VROViewUniforms *viewUniforms = (VROViewUniforms *)_viewUniformsBuffer->getWritableContents(eyeType, frame);
        
        viewUniforms->normal_matrix = toMatrixFloat4x4(transform.invert().transpose());
        viewUniforms->model_matrix = toMatrixFloat4x4(transform);
        viewUniforms->modelview_matrix = toMatrixFloat4x4(modelview);
        viewUniforms->modelview_projection_matrix = toMatrixFloat4x4(projectionMatrix.multiply(modelview));
        viewUniforms->camera_position = toVectorFloat3(renderContext.getCamera().getPosition());
         */
        _program->getUniform(0)->set(projectionMatrix.multiply(modelview).getArray());
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element.buffer);
        glDrawElements(element.primitiveType, element.indexCount, element.indexType, 0);
        
        
        //[renderEncoder setVertexBuffer:_viewUniformsBuffer->getMTLBuffer(eyeType)
        //                        offset:_viewUniformsBuffer->getWriteOffset(frame) atIndex:_vars.size()];
        
        /*
         Determine if the material has been updated. If so, we need to update our pipeline and
         depth states.
         */
        /*
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
            
            renderMaterial(outgoingSubstrate, element, outgoingPipelineState, depthState, renderEncoder, params,
                           renderContext, driver);
            renderMaterial(substrate, element, pipelineState, depthState, renderEncoder, params,
                           renderContext, driver);
        }
        else {
            _outgoingPipelineStates[i] = nullptr;
            renderMaterial(substrate, element, pipelineState, depthState, renderEncoder, params,
                           renderContext, driver);
        }
        */
        pglpop();
    }
}

/*
void VROGeometrySubstrateOpenGL::renderMaterial(VROMaterialSubstrateMetal *material,
                                                VROGeometryElementMetal &element,
                                                id <MTLRenderPipelineState> pipelineState,
                                                id <MTLDepthStencilState> depthStencilState,
                                                id <MTLRenderCommandEncoder> renderEncoder,
                                                VRORenderParameters &params,
                                                const VRORenderContext &renderContext,
                                                const VRODriver &driver) {
    
    int frame = renderContext.getFrame();
    VROEyeType eyeType = renderContext.getEyeType();
    
    [renderEncoder setRenderPipelineState:pipelineState];
    [renderEncoder setDepthStencilState:depthStencilState];
    
    VROConcurrentBuffer &materialBuffer = material->bindMaterialUniforms(params, eyeType, frame);
    [renderEncoder setVertexBuffer:materialBuffer.getMTLBuffer(eyeType)
                            offset:materialBuffer.getWriteOffset(frame)
                           atIndex:_vars.size() + 1];
    
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
*/