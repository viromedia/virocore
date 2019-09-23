//
//  VROGeometrySubstrateOpenGL.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
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
#include "VROTextureReference.h"
#include "VROVertexBufferOpenGL.h"
#include <map>

VROGeometrySubstrateOpenGL::VROGeometrySubstrateOpenGL(const VROGeometry &geometry,
                                                       std::shared_ptr<VRODriverOpenGL> driver) :
    _driver(driver) {
    readGeometryElements(geometry.getGeometryElements());
        
    std::vector<std::shared_ptr<VROGeometrySource>> sources = geometry.getGeometrySources();
    if (geometry.getSkinner()) {
        _boneUBO = std::unique_ptr<VROBoneUBO>(new VROBoneUBO(driver));

        // For glTF, bone and weights data are already processed into geometry sources.
        if (geometry.getSkinner()->getBoneIndices() != nullptr) {
            sources.push_back(geometry.getSkinner()->getBoneIndices());
            sources.push_back(geometry.getSkinner()->getBoneWeights());
        }
    }
    readGeometrySources(sources);
        
    createVAO();
}

VROGeometrySubstrateOpenGL::~VROGeometrySubstrateOpenGL() {
    // Ensure we are deleting GL objects with the current GL context
    std::shared_ptr<VRODriverOpenGL> driver = _driver.lock();
    if (driver) {
        for (VROGeometryElementOpenGL &element : _elements) {
            driver->deleteBuffer(element.buffer);
        }
        for (VROVertexDescriptorOpenGL &vd : _vertexDescriptors) {
            if (vd.ownsBuffer) {
                driver->deleteBuffer(vd.buffer);
                ALLOCATION_TRACKER_SUB(VBO, 1);
            }
        }
        for (GLuint vao : _vaos) {
            driver->deleteVertexArray(vao);
        }
    } else {
        // Make the allocation tracker subtract VBOs if the driver was
        // released, just to keep the counter accurate.
        for (VROVertexDescriptorOpenGL &vd : _vertexDescriptors) {
            if (vd.ownsBuffer) {
                ALLOCATION_TRACKER_SUB(VBO, 1);
            }
        }
    }

    std::map<int, std::vector<VROVertexDescriptorOpenGL>>::iterator it;
    for (it = _elementToDescriptorsMap.begin(); it != _elementToDescriptorsMap.end(); it++) {
        it->second.clear();
    }
    _elementToDescriptorsMap.clear();
    _elements.clear();
}

void VROGeometrySubstrateOpenGL::readGeometryElements(const std::vector<std::shared_ptr<VROGeometryElement>> &elements) {
    for (std::shared_ptr<VROGeometryElement> element : elements) {
        if (element->getData() == nullptr) {
            continue;
        }
        VROGeometryElementOpenGL elementOGL;
        
        int indexCount = VROGeometryUtilGetIndicesCount(element->getPrimitiveCount(), element->getPrimitiveType());
        
        GL( glGenBuffers(1, &elementOGL.buffer) );
        GL( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementOGL.buffer) );
        GL( glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * element->getBytesPerIndex(), element->getData()->getData(), GL_STATIC_DRAW) );
     
        elementOGL.primitiveType = parsePrimitiveType(element->getPrimitiveType());
        elementOGL.indexCount = indexCount;
        elementOGL.indexType = (element->getBytesPerIndex() == 2) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
        elementOGL.indexBufferOffset = 0;
        _elements.push_back(elementOGL);
    }
}

void VROGeometrySubstrateOpenGL::readGeometrySources(const std::vector<std::shared_ptr<VROGeometrySource>> &sources) {
    std::map<std::shared_ptr<VROData>, std::vector<std::shared_ptr<VROGeometrySource>>> dataMap;
    std::map<std::shared_ptr<VROVertexBuffer>, std::vector<std::shared_ptr<VROGeometrySource>>> vboMap;

    /*
     Sort the sources into groups defined by the data buffer (or vertex buffer) they're using.
     */
    for (std::shared_ptr<VROGeometrySource> source : sources) {
        std::shared_ptr<VROVertexBuffer> vertexBuffer = source->getVertexBuffer();
        if (vertexBuffer) {
            vboMap[vertexBuffer].push_back(source);
        } else {
            std::shared_ptr<VROData> data = source->getData();
            if (data && data->getDataLength() > 0) {
                dataMap[data].push_back(source);
            }
        }
    }
    
    /*
     Upload VBOs to the GPU (if they have not been already), and create vertex descriptors
     defining the attributes over the buffer.
     */
    for (auto &kv : vboMap) {
        std::shared_ptr<VROVertexBufferOpenGL> vbo = std::dynamic_pointer_cast<VROVertexBufferOpenGL>(kv.first);
        std::vector<std::shared_ptr<VROGeometrySource>> group = kv.second;
        
        vbo->hydrate();
        
        VROVertexDescriptorOpenGL vd = configureVertexDescriptor(vbo->getVBO(), group);
        vd.ownsBuffer = false;
        _vertexDescriptors.push_back(vd);
    }
    
    /*
     Do the same for the raw CPU data buffers, uploading them to the GPU and assigning
     vertex descriptors.
     */
    for (auto &kv : dataMap) {
        std::shared_ptr<VROData> data = kv.first;
        std::vector<std::shared_ptr<VROGeometrySource>> group = kv.second;
        
        GLuint buffer;
        GL( glGenBuffers(1, &buffer) );
        GL( glBindBuffer(GL_ARRAY_BUFFER, buffer) );
        GL( glBufferData(GL_ARRAY_BUFFER, data->getDataLength(), data->getData(), GL_STATIC_DRAW) );
        
        ALLOCATION_TRACKER_ADD(VBO, 1);
        
        VROVertexDescriptorOpenGL vd = configureVertexDescriptor(buffer, group);
        vd.ownsBuffer = true;
        _vertexDescriptors.push_back(vd);
    }
}

VROVertexDescriptorOpenGL VROGeometrySubstrateOpenGL::configureVertexDescriptor(GLuint buffer, std::vector<std::shared_ptr<VROGeometrySource>> group) {
    VROVertexDescriptorOpenGL vd;
    vd.stride = group[0]->getDataStride();
    vd.numAttributes = 0;
    vd.buffer = buffer;
    
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
        
        int elementIndex = source->getGeometryElementIndex();
        if (elementIndex != -1) {
            if (_elementToDescriptorsMap.find(elementIndex) == _elementToDescriptorsMap.end()) {
                _elementToDescriptorsMap[elementIndex] = std::vector<VROVertexDescriptorOpenGL>();
            }
            _elementToDescriptorsMap[elementIndex].push_back(vd);
        }
    }
    return vd;
}

void VROGeometrySubstrateOpenGL::createVAO() {
    GLuint vaos[_elements.size()];
    GL( glGenVertexArrays((int) _elements.size(), vaos) );
    
    for (int i = 0; i < _elements.size(); i++) {
        GL( glBindVertexArray(vaos[i]) );
        std::vector<VROVertexDescriptorOpenGL> vertexDescriptors = _vertexDescriptors;
        if (_elementToDescriptorsMap.size() > 0
                && _elementToDescriptorsMap.find(i) != _elementToDescriptorsMap.end()) {
            vertexDescriptors = _elementToDescriptorsMap[i];
        }

        for (VROVertexDescriptorOpenGL &vd : vertexDescriptors) {
            GL( glBindBuffer(GL_ARRAY_BUFFER, vd.buffer) );
    
            for (int i = 0; i < vd.numAttributes; i++) {
                if (vd.attributes[i].type == GL_INT || vd.attributes[i].type == GL_SHORT) {
                    GL( glVertexAttribIPointer(vd.attributes[i].index, vd.attributes[i].size, vd.attributes[i].type, vd.stride,
                                               (GLvoid *) vd.attributes[i].offset) );
                }
                else {
                    GL( glVertexAttribPointer(vd.attributes[i].index, vd.attributes[i].size, vd.attributes[i].type, GL_FALSE, vd.stride,
                                              (GLvoid *) vd.attributes[i].offset) );
                }
                GL( glEnableVertexAttribArray(vd.attributes[i].index) );
            }
        }
        
        GL( glBindBuffer(GL_ARRAY_BUFFER, 0) );
        GL( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elements[i].buffer) );
        GL( glBindVertexArray(0) );
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

void VROGeometrySubstrateOpenGL::update(const VROGeometry &geometry,
                                        std::shared_ptr<VRODriver> &driver) {
    if (_boneUBO) {
        _boneUBO->update(geometry.getSkinner());
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
         Screen space geometries are specified in viewport (screen) coordinates.
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
    
    if (_boneUBO) {
        _boneUBO->bind();
    }

    VROGeometryElementOpenGL element = _elements[elementIndex];
    VROMaterialSubstrateOpenGL *substrate = static_cast<VROMaterialSubstrateOpenGL *>(material->getSubstrate(driver));
    substrate->bindView(transform, viewMatrix, projectionMatrix, normalMatrix,
                        context.getCamera().getPosition(), context.getEyeType());
    
    passert (elementIndex < _vaos.size() && elementIndex >= 0);
    GL (glBindVertexArray(_vaos[elementIndex]) );
    renderMaterial(geometry, material, substrate, element, opacity, context, driver);
    GL (glBindVertexArray(0) );
    
    pglpop();
}

void VROGeometrySubstrateOpenGL::renderMaterial(const VROGeometry &geometry,
                                                const std::shared_ptr<VROMaterial> &material,
                                                VROMaterialSubstrateOpenGL *substrate,
                                                VROGeometryElementOpenGL &element,
                                                float opacity,
                                                const VRORenderContext &context,
                                                std::shared_ptr<VRODriver> &driver) {
    substrate->bindGeometry(opacity, geometry);

    int activeTexture = 0;
    const std::vector<VROTextureReference> &textureReferences = substrate->getTextures();
    for (int j = 0; j < textureReferences.size(); ++j) {
        const VROTextureReference &reference = textureReferences[j];
        
        // TODO Remove this hack, which eliminates shadows for materials that do not have receivesShadows
        //      set. Instead, material.getReceivesShadows should factor into the capabilities key for the
        //      shader.
        if (reference.isGlobal() && reference.getGlobalType() == VROGlobalTextureType::ShadowMap && !material->getReceivesShadows()) {
            ++activeTexture;
            continue;
        }

        const std::shared_ptr<VROTexture> &texture = reference.getTexture(context);

        for (int s = 0; s < texture->getNumSubstrates(); s++) {
            VROTextureSubstrateOpenGL *substrate = (VROTextureSubstrateOpenGL *) texture->getSubstrate(s, driver, false);
            if (!substrate) {
                // Use a blank placeholder if a texture is not yet available (i.e.
                // during video texture loading)
                std::shared_ptr<VROTexture> blank = getBlankTexture(texture->getType());
                substrate = (VROTextureSubstrateOpenGL *) blank->getSubstrate(0, driver, true);
            }
            
            std::pair<GLenum, GLuint> targetAndTexture = substrate->getTexture();
            driver->bindTexture(GL_TEXTURE0 + activeTexture, targetAndTexture.first, targetAndTexture.second);
            ++activeTexture;
        }
    }

    const std::shared_ptr<VROInstancedUBO> &instancedUBO = geometry.getInstancedUBO();
    if (instancedUBO != nullptr) {
        int numberOfDraws = instancedUBO->getNumberOfDrawCalls();
        for (int i = 0; i < numberOfDraws; i++) {
            int instances = instancedUBO->bindDrawData(i);
            GL( glDrawElementsInstanced(element.primitiveType, element.indexCount, element.indexType, 0, instances) );
        }
    }
    else {
        GL( glDrawElements(element.primitiveType, element.indexCount, element.indexType, 0) );
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
        if (_boneUBO) {
            _boneUBO->bind();
        }
        
        VROMaterialSubstrateOpenGL *substrate = static_cast<VROMaterialSubstrateOpenGL *>(material->getSubstrate(driver));
        substrate->bindView(transform, viewMatrix, projectionMatrix, normalMatrix,
                            context.getCamera().getPosition(), context.getEyeType());
        
        GL( glBindVertexArray(_vaos[i]) );
        substrate->bindGeometry(1.0, geometry);
        GL( glDrawElements(element.primitiveType, element.indexCount, element.indexType, 0) );
        GL( glBindVertexArray(0) );
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
    
    if (_boneUBO) {
        _boneUBO->bind();
    }

    VROGeometryElementOpenGL &element = _elements[elementIndex];
    VROMaterialSubstrateOpenGL *substrate = static_cast<VROMaterialSubstrateOpenGL *>(material->getSubstrate(driver));
    substrate->bindView(transform, viewMatrix, projectionMatrix, normalMatrix,
                        context.getCamera().getPosition(), context.getEyeType());
    
    GL( glBindVertexArray(_vaos[elementIndex]) );
    renderMaterial(geometry, material, substrate, element, 1.0, context, driver);
    GL( glBindVertexArray(0) );
    
    pglpop();
}
