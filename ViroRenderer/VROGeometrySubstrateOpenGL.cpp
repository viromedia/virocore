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
}

VROGeometrySubstrateOpenGL::~VROGeometrySubstrateOpenGL() {
    
}

void VROGeometrySubstrateOpenGL::readGeometryElements(const std::vector<std::shared_ptr<VROGeometryElement>> &elements) {
    
    for (std::shared_ptr<VROGeometryElement> element : elements) {
        VROGeometryElementOpenGL elementOGL;
        
        int indexCount = VROGeometryUtilGetIndicesCount(element->getPrimitiveCount(), element->getPrimitiveType());
        
        glGenBuffers(1, &elementOGL.buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementOGL.buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * element->getBytesPerIndex(), element->getData()->getData(), GL_STATIC_DRAW);
     
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
    
    /*
     For each group of GeometrySources we create an MTLBuffer and layout.
     */
    for (auto &kv : dataMap) {
        std::vector<std::shared_ptr<VROGeometrySource>> group = kv.second;
        
        /*
         Create an MTLBuffer that wraps over the VROData.
         */
        int dataSize = 0;
        for (std::shared_ptr<VROGeometrySource> source : group) {
            int size = source->getVertexCount() * source->getDataStride();
            dataSize = std::max(dataSize, size);
        }
        
        VROVertexDescriptorOpenGL vd;
        vd.stride = group[0]->getDataStride();
        vd.numAttributes = 0;
        
        glGenBuffers(1, &vd.buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vd.buffer);
        glBufferData(GL_ARRAY_BUFFER, dataSize, kv.first->getData(), GL_STATIC_DRAW);
        
        /*
         Create an attribute for each geometry source in this group.
         */
        for (int i = 0; i < group.size(); i++) {
            std::shared_ptr<VROGeometrySource> source = group[i];
            int attrIdx = VROGeometryUtilParseAttributeIndex(source->getSemantic());
            
            std::pair<GLuint, int> format = parseVertexFormat(source);
            vd.attributes[vd.numAttributes].index = attrIdx;
            vd.attributes[vd.numAttributes].size = format.second;
            vd.attributes[vd.numAttributes].type = format.first;
            vd.attributes[vd.numAttributes].offset = source->getDataOffset();
            
            vd.numAttributes++;
            passert (source->getDataStride() == vd.stride);
        }
        
        _vertexDescriptor.push_back(vd);
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
         Determine if the material has been updated. If so, we need to update our pipeline and
         depth states.
         */
         const std::shared_ptr<VROMaterial> &material = materials[i % materials.size()];
        /*
         if (material->isUpdated()) {
         _elementPipelineStates[i] = createRenderPipelineState(material, metal);
         _elementDepthStates[i] = createDepthStencilState(material, metal.getDevice());
         }
         */
        
        material->createSubstrate(driver);
        VROMaterialSubstrateOpenGL *substrate = static_cast<VROMaterialSubstrateOpenGL *>(material->getSubstrate());
        
        /*
         Configure the view uniforms.
         */
        VROMatrix4f modelview = viewMatrix.multiply(transform);
        substrate->bindShader();
        substrate->bindViewUniforms(transform, modelview, projectionMatrix, renderContext.getCamera().getPosition());
        substrate->bindLightingUniforms(params.lights, eyeType, frame);
        
        for (VROVertexDescriptorOpenGL &vd : _vertexDescriptor) {
            glBindBuffer(GL_ARRAY_BUFFER, vd.buffer);
           
            for (int i = 0; i < vd.numAttributes; i++) {
                glVertexAttribPointer(vd.attributes[i].index, vd.attributes[i].size, vd.attributes[i].type, GL_FALSE, vd.stride, (GLvoid *) vd.attributes[i].offset);
                glEnableVertexAttribArray(vd.attributes[i].index);
            }
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element.buffer);
        
        const std::shared_ptr<VROMaterial> &outgoing = material->getOutgoing();
        if (outgoing) {
            outgoing->createSubstrate(driver);
            VROMaterialSubstrateOpenGL *outgoingSubstrate = static_cast<VROMaterialSubstrateOpenGL *>(outgoing->getSubstrate());
            
            renderMaterial(outgoingSubstrate, element, params,
                           renderContext, driver);
            renderMaterial(substrate, element, params,
                           renderContext, driver);
        }
        else {
            renderMaterial(substrate, element, params,
                           renderContext, driver);
        }
        
        pglpop();
    }
}

void VROGeometrySubstrateOpenGL::renderMaterial(VROMaterialSubstrateOpenGL *material,
                                                VROGeometryElementOpenGL &element,
                                                VRORenderParameters &params,
                                                const VRORenderContext &renderContext,
                                                const VRODriver &driver) {
    
    int frame = renderContext.getFrame();
    VROEyeType eyeType = renderContext.getEyeType();
    
    material->bindMaterialUniforms(params, eyeType, frame);
    
    const std::vector<std::shared_ptr<VROTexture>> &textures = material->getTextures();
    for (int j = 0; j < textures.size(); ++j) {
        VROTextureSubstrateOpenGL *substrate = (VROTextureSubstrateOpenGL *) textures[j]->getSubstrate(driver);
        if (!substrate) {
            // Use a blank placeholder if a texture is not yet available (i.e.
            // during video texture loading)
            std::shared_ptr<VROTexture> blank = getBlankTexture();
            substrate = (VROTextureSubstrateOpenGL *) blank->getSubstrate(driver);
        }
        
        std::pair<GLenum, GLint> targetAndTexture = substrate->getTexture();
        
        glActiveTexture(GL_TEXTURE0 + j);
        glBindTexture(targetAndTexture.first, targetAndTexture.second);
    }
    
    glDrawElements(element.primitiveType, element.indexCount, element.indexType, 0);
}