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
#include "VROBoneUBO.h"
#include "VROInstancedUBO.h"
#include "VROShaderProgram.h"
#include <map>

VROGeometrySubstrateOpenGL::VROGeometrySubstrateOpenGL(const VROGeometry &geometry,
                                                       std::shared_ptr<VRODriverOpenGL> driver) :
    _driver(driver) {
    
    readGeometryElements(geometry.getGeometryElements());
        
    std::vector<std::shared_ptr<VROGeometrySource>> sources = geometry.getGeometrySources();
    if (geometry.getSkinner()) {
        sources.push_back(geometry.getSkinner()->getBoneIndices());
        sources.push_back(geometry.getSkinner()->getBoneWeights());
        
        _boneUBO = std::unique_ptr<VROBoneUBO>(new VROBoneUBO(driver));
    }
    readGeometrySources(sources);
        
    createVAO();
}

VROGeometrySubstrateOpenGL::~VROGeometrySubstrateOpenGL() {
    std::vector<GLuint> buffers;
    for (VROGeometryElementOpenGL &element : _elements) {
        buffers.push_back(element.buffer);
    }
    for (VROVertexDescriptorOpenGL &vd : _vertexDescriptors) {
        buffers.push_back(vd.buffer);
    }

    // Ensure we are deleting GL objects with the current GL context
    if (_driver.lock()) {
        glDeleteBuffers((int) buffers.size(), buffers.data());
        glDeleteVertexArrays((int) _vaos.size(), _vaos.data());
    }
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
     For each group of GeometrySources we create a VROVertexDescriptorOpenGL.
     */
    for (auto &kv : dataMap) {
        std::vector<std::shared_ptr<VROGeometrySource>> group = kv.second;
        
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
        
        _vertexDescriptors.push_back(vd);
    }
}

void VROGeometrySubstrateOpenGL::createVAO() {
    GLuint vaos[_elements.size()];
    glGenVertexArrays((int) _elements.size(), vaos);
    
    for (int i = 0; i < _elements.size(); i++) {
        glBindVertexArray(vaos[i]);
        
        for (VROVertexDescriptorOpenGL &vd : _vertexDescriptors) {
            glBindBuffer(GL_ARRAY_BUFFER, vd.buffer);
            
            for (int i = 0; i < vd.numAttributes; i++) {
                if (vd.attributes[i].type == GL_INT || vd.attributes[i].type == GL_SHORT) {
                    glVertexAttribIPointer(vd.attributes[i].index, vd.attributes[i].size, vd.attributes[i].type, vd.stride,
                                           (GLvoid *) vd.attributes[i].offset);
                }
                else {
                    glVertexAttribPointer(vd.attributes[i].index, vd.attributes[i].size, vd.attributes[i].type, GL_FALSE, vd.stride,
                                          (GLvoid *) vd.attributes[i].offset);
                }
                glEnableVertexAttribArray(vd.attributes[i].index);
            }
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elements[i].buffer);
        glBindVertexArray(0);
    }
    
    _vaos.assign(vaos, vaos + _elements.size());
}

std::pair<GLuint, int> VROGeometrySubstrateOpenGL::parseVertexFormat(std::shared_ptr<VROGeometrySource> &source) {
    if (source->isFloatComponents()) {
        switch (source->getBytesPerComponent()) {
            case 2:
                return { GL_HALF_FLOAT, source->getComponentsPerVertex() };
            case 4:
                return { GL_FLOAT, source->getComponentsPerVertex() };
            default:
                pabort();
                return { GL_FLOAT, 1 };
        }
    }
    else {
        switch (source->getBytesPerComponent()) {
            case 2:
                return { GL_SHORT, source->getComponentsPerVertex() };
            case 4:
                return { GL_INT, source->getComponentsPerVertex() };
            default:
                pabort();
                return { GL_INT, 1 };
        }
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
                                        int elementIndex,
                                        VROMatrix4f transform,
                                        VROMatrix4f normalMatrix,
                                        float opacity,
                                        const std::shared_ptr<VROMaterial> &material,
                                        const VRORenderContext &context,
                                        std::shared_ptr<VRODriver> &driver) {
    
    VROMatrix4f viewMatrix = context.getViewMatrix();
    VROMatrix4f projectionMatrix = context.getProjectionMatrix();
    
    if (geometry.isCameraEnclosure()) {
        viewMatrix = context.getEnclosureViewMatrix();
    }
    if (geometry.isScreenSpace()) {
        /*
         Screen space geometries are specified in viepwort (screen) coordinates.
         Therefore they do not respond to the camera (identity view matrix), and
         they use an orthographic projection.
         */
        viewMatrix = VROMatrix4f();
        projectionMatrix = context.getOrthographicMatrix();
    }
    
    std::string geoName = geometry.getName();
    std::string materialName = material->getName();
    
    if (!materialName.empty()) {
        pglpush("Geometry [%s], Material [%s]", geoName.c_str(), materialName.c_str());
    }
    else {
        pglpush("Geometry [%s]", geoName.c_str());
    }
    
    VROGeometryElementOpenGL element = _elements[elementIndex];
    
    VROMaterialSubstrateOpenGL *substrate = static_cast<VROMaterialSubstrateOpenGL *>(material->getSubstrate(driver));
    if (_boneUBO) {
        _boneUBO->update(geometry.getSkinner());
        substrate->bindBoneUBO(_boneUBO);
    }

    const std::shared_ptr<VROInstancedUBO> &instancedUBO = geometry.getInstancedUBO();
    if (instancedUBO != nullptr) {
        substrate->bindInstanceUBO(instancedUBO);
    }
    substrate->bindView(transform, viewMatrix, projectionMatrix, normalMatrix,
                        context.getCamera().getPosition(), context.getEyeType());
    
    glBindVertexArray(_vaos[elementIndex]);
    renderMaterial(geometry, substrate, element, opacity, context, driver);
    glBindVertexArray(0);
    
    pglpop();
}

void VROGeometrySubstrateOpenGL::renderMaterial(const VROGeometry &geometry,
                                                VROMaterialSubstrateOpenGL *material,
                                                VROGeometryElementOpenGL &element,
                                                float opacity,
                                                const VRORenderContext &context,
                                                std::shared_ptr<VRODriver> &driver) {
    material->bindGeometry(opacity, geometry);

    int activeTexture = 0;
    const std::vector<std::shared_ptr<VROTexture>> &textures = material->getTextures();
    for (int j = 0; j < textures.size(); ++j) {
        const std::shared_ptr<VROTexture> &texture = textures[j];
        
        for (int s = 0; s < texture->getNumSubstrates(); s++) {
            VROTextureSubstrateOpenGL *substrate = (VROTextureSubstrateOpenGL *) texture->getSubstrate(s, driver, context.getFrameScheduler().get());
            if (!substrate) {
                // Use a blank placeholder if a texture is not yet available (i.e.
                // during video texture loading)
                std::shared_ptr<VROTexture> blank = getBlankTexture();
                substrate = (VROTextureSubstrateOpenGL *) blank->getSubstrate(0, driver, nullptr);
            }
            
            std::pair<GLenum, GLuint> targetAndTexture = substrate->getTexture();
            
            glActiveTexture(GL_TEXTURE0 + activeTexture);
            glBindTexture(targetAndTexture.first, targetAndTexture.second);
            
            ++activeTexture;
        }
    }

    if (context.getShadowMap()) {
        VROTextureSubstrateOpenGL *substrate = (VROTextureSubstrateOpenGL *) context.getShadowMap()->getSubstrate(0, driver, nullptr);
        std::pair<GLenum, GLuint> targetAndTexture = substrate->getTexture();
        
        glActiveTexture(GL_TEXTURE0 + activeTexture);
        glBindTexture(targetAndTexture.first, targetAndTexture.second);
    }

    const std::shared_ptr<VROInstancedUBO> &instancedUBO = geometry.getInstancedUBO();
    if (instancedUBO != nullptr) {
        int numberOfDraws = instancedUBO->getNumberOfDrawCalls();
        for (int i = 0; i < numberOfDraws; i ++) {
            int instances = instancedUBO->bindDrawData(i);
            glDrawElementsInstanced(element.primitiveType, element.indexCount, element.indexType, 0, instances);
        }
    }
    else {
        glDrawElements(element.primitiveType, element.indexCount, element.indexType, 0);
    }
}

void VROGeometrySubstrateOpenGL::renderSilhouette(const VROGeometry &geometry,
                                                  VROMatrix4f transform,
                                                  std::shared_ptr<VROMaterial> &material,
                                                  const VRORenderContext &context,
                                                  std::shared_ptr<VRODriver> &driver) {
    
    VROMatrix4f viewMatrix = context.getViewMatrix();
    VROMatrix4f projectionMatrix = context.getProjectionMatrix();
    VROMatrix4f normalMatrix; // Silhouettes ignore lighting so normal matrix can be identity

    if (geometry.isCameraEnclosure()) {
        viewMatrix = context.getEnclosureViewMatrix();
    }
    if (geometry.isScreenSpace()) {
        viewMatrix = VROMatrix4f();
        projectionMatrix = context.getOrthographicMatrix();
    }
    
    pglpush("Silhouette [%s]", geometry.getName().c_str());
    for (int i = 0; i < geometry.getGeometryElements().size(); i++) {
        VROGeometryElementOpenGL &element = _elements[i];
        
        VROMaterialSubstrateOpenGL *substrate = static_cast<VROMaterialSubstrateOpenGL *>(material->getSubstrate(driver));
        if (_boneUBO) {
            _boneUBO->update(geometry.getSkinner());
            substrate->bindBoneUBO(_boneUBO);
        }
        
        substrate->bindView(transform, viewMatrix, projectionMatrix, normalMatrix,
                            context.getCamera().getPosition(), context.getEyeType());
        
        glBindVertexArray(_vaos[i]);
        substrate->bindGeometry(1.0, geometry);
        glDrawElements(element.primitiveType, element.indexCount, element.indexType, 0);
        glBindVertexArray(0);
    }
    pglpop();
}

void VROGeometrySubstrateOpenGL::renderSilhouetteTextured(const VROGeometry &geometry,
                                                          int elementIndex,
                                                          VROMatrix4f transform,
                                                          std::shared_ptr<VROMaterial> &material,
                                                          const VRORenderContext &context,
                                                          std::shared_ptr<VRODriver> &driver) {
    
    VROMatrix4f viewMatrix = context.getViewMatrix();
    VROMatrix4f projectionMatrix = context.getProjectionMatrix();
    VROMatrix4f normalMatrix; // Silhouettes ignore lighting so normal matrix can be identity
    
    if (geometry.isCameraEnclosure()) {
        viewMatrix = context.getEnclosureViewMatrix();
    }
    if (geometry.isScreenSpace()) {
        viewMatrix = VROMatrix4f();
        projectionMatrix = context.getOrthographicMatrix();
    }
    
    pglpush("Silhouette [%s]", geometry.getName().c_str());
    
    VROGeometryElementOpenGL &element = _elements[elementIndex];
    VROMaterialSubstrateOpenGL *substrate = static_cast<VROMaterialSubstrateOpenGL *>(material->getSubstrate(driver));
    if (_boneUBO) {
        _boneUBO->update(geometry.getSkinner());
        substrate->bindBoneUBO(_boneUBO);
    }
    
    substrate->bindView(transform, viewMatrix, projectionMatrix, normalMatrix,
                        context.getCamera().getPosition(), context.getEyeType());
    
    glBindVertexArray(_vaos[elementIndex]);
    renderMaterial(geometry, substrate, element, 1.0, context, driver);
    glBindVertexArray(0);
    
    pglpop();
}
