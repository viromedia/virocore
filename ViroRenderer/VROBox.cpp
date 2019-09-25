//
//  VROBox.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/7/15.
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

#include "VROBox.h"
#include "VROData.h"
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"
#include "VROMaterial.h"
#include "VROLog.h"
#include "VROAnimationFloat.h"
#include "stdlib.h"

static const int kNumBoxVertices = 36;

std::shared_ptr<VROBox> VROBox::createBox(float width, float height, float length) {
    return std::shared_ptr<VROBox>(new VROBox(width, height, length));
}

VROBox::VROBox(float width, float height, float length) :
    _width(width), _height(height), _length(length) {
    
    updateBox();
}

VROBox::~VROBox() {
    
}

void VROBox::updateBox() {
    // If there are no materials, set a default one
    if (getMaterials().empty()) {
        std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
        VROGeometry::setMaterials({ material });
    }
    
    int numVertices = kNumBoxVertices;
    
    int varSizeBytes = sizeof(VROShapeVertexLayout) * numVertices;
    VROShapeVertexLayout var[varSizeBytes];
    buildBoxVAR(var);
    
    int indices[kNumBoxVertices];
    for (int i = 0; i < kNumBoxVertices; i++) {
        indices[i] = i;
    }
    VROShapeUtilComputeTangents(var, numVertices, indices, numVertices);
    
    std::shared_ptr<VROData> vertexData = std::make_shared<VROData>((void *) var, varSizeBytes);
    
    std::vector<std::shared_ptr<VROGeometrySource>> sources = VROShapeUtilBuildGeometrySources(vertexData, numVertices);
    setSources(sources);
    
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    if (getMaterials().size() == 6) {
        for (int i = 0; i < 6; i++) {
            std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) (indices + i * 6), sizeof(int) * kNumBoxVertices / 6);
            std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(indexData,
                                                                                               VROGeometryPrimitiveType::Triangle,
                                                                                               (kNumBoxVertices / 3) / 6,
                                                                                               sizeof(int));
            elements.push_back(element);
        }
    }
    else {
        std::shared_ptr<VROData> indexData = std::make_shared<VROData>((void *) indices, sizeof(int) * kNumBoxVertices);
        std::shared_ptr<VROGeometryElement> element = std::make_shared<VROGeometryElement>(indexData,
                                                                                           VROGeometryPrimitiveType::Triangle,
                                                                                           kNumBoxVertices / 3,
                                                                                           sizeof(int));
        elements.push_back(element);
    }
    setElements(elements);
    updateBoundingBox();
}

void VROBox::buildBoxVAR(VROShapeVertexLayout *vertexLayout) {
    float w = _width  / 2;
    float h = _height / 2;
    float l = _length / 2;
    float u = 1;
    float v = 1;
    
    const float cubeVertices[] = {
        // Front face
        -w,  h, l,
        -w, -h, l,
        w,  h, l,
        -w, -h, l,
        w, -h, l,
        w,  h, l,
        
        // Right face
        w,  h,  l,
        w, -h,  l,
        w,  h, -l,
        w, -h,  l,
        w, -h, -l,
        w,  h, -l,
        
        // Back face
        w,  h, -l,
        w, -h, -l,
        -w,  h, -l,
        w, -h, -l,
        -w, -h, -l,
        -w,  h, -l,
        
        // Left face
        -w,  h, -l,
        -w, -h, -l,
        -w,  h,  l,
        -w, -h, -l,
        -w, -h,  l,
        -w,  h,  l,
        
        // Top face
        -w, h, -l,
        -w, h,  l,
        w, h, -l,
        -w, h,  l,
        w, h,  l,
        w, h, -l,
        
        // Bottom face
        w, -h, -l,
        w, -h,  l,
        -w, -h, -l,
        w, -h,  l,
        -w, -h,  l,
        -w, -h, -l,
    };
    
    const float cubeTex[] = {
        // Front face
        0, 0,
        0, v,
        u, 0,
        0, v,
        u, v,
        u, 0,
        
        // Right face
        0, 0,
        0, v,
        u, 0,
        0, v,
        u, v,
        u, 0,
        
        // Back face
        0, 0,
        0, v,
        u, 0,
        0, v,
        u, v,
        u, 0,
        
        // Left face
        0, 0,
        0, v,
        u, 0,
        0, v,
        u, v,
        u, 0,
        
        // Top face
        0, 0,
        0, v,
        u, 0,
        0, v,
        u, v,
        u, 0,
        
        // Bottom face
        0, 0,
        0, v,
        u, 0,
        0, v,
        u, v,
        u, 0,
    };
    
    const float cubeNormals[] = {
        // Front face
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        
        // Right face
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        
        // Back face
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        
        // Left face
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        
        // Top face
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        
        // Bottom face
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f
    };
    
    for (int i = 0; i < kNumBoxVertices; i++) {
        vertexLayout[i].x =  cubeVertices[i * 3 + 0];
        vertexLayout[i].y =  cubeVertices[i * 3 + 1];
        vertexLayout[i].z =  cubeVertices[i * 3 + 2];
        vertexLayout[i].u =  cubeTex[i * 2 + 0];
        vertexLayout[i].v =  cubeTex[i * 2 + 1];
        vertexLayout[i].nx = cubeNormals[i * 3 + 0];
        vertexLayout[i].ny = cubeNormals[i * 3 + 1];
        vertexLayout[i].nz = cubeNormals[i * 3 + 2];
    }
}

void VROBox::setWidth(float width) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float v) {
        ((VROBox *)animatable)->_width = v;
        ((VROBox *)animatable)->updateBox();
    }, _width, width));
}

void VROBox::setHeight(float height) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float v) {
        ((VROBox *)animatable)->_height = v;
        ((VROBox *)animatable)->updateBox();
    }, _height, height));
}

void VROBox::setLength(float length) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float v) {
        ((VROBox *)animatable)->_length = v;
        ((VROBox *)animatable)->updateBox();
    }, _length, length));
}

void VROBox::setMaterials(std::vector<std::shared_ptr<VROMaterial>> materials) {
    if ((getMaterials().size() == 6 && materials.size() != 6) ||
        (getMaterials().size() == 1 && materials.size() != 1)) {
        
        VROGeometry::setMaterials(materials);
        updateBox();
    }
    else {
        VROGeometry::setMaterials(materials);
    }
}

