//
//  VROGeometryElement.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
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

#include "VROGeometryElement.h"
#include "VROByteBuffer.h"
#include "VROGeometrySource.h"
#include "VROLog.h"
#include "VROMath.h"

void VROGeometryElement::processTriangles(std::function<void(int index, VROTriangle triangle)> function,
                                          std::shared_ptr<VROGeometrySource> geometrySource) const {
    
    std::vector<VROVector3f> vertices;
    geometrySource->processVertices([&vertices](int index, VROVector4f vertex) {
        vertices.push_back(VROVector3f(vertex.x, vertex.y, vertex.z));
    });
    
    VROVector3f A, B, C;

    if (_primitiveType == VROGeometryPrimitiveType::Triangle) {
        processIndices([&vertices, &A, &B, &C, function](int index, int indexRead) {
            if (index % 3 == 0) {
                A = vertices[indexRead];
            }
            if (index % 3 == 1) {
                B = vertices[indexRead];
            }
            if (index % 3 == 2) {
                C = vertices[indexRead];
                function(index / 3, {A, B, C});
            }
        });
    }
    else if (_primitiveType == VROGeometryPrimitiveType::TriangleStrip) {
        processIndices([&vertices, &A, &B, &C, function](int index, int indexRead) {
            if (index == 0) {
                A = vertices[indexRead];
            }
            else if (index == 1) {
                B = vertices[indexRead];
            }
            else {
                C = vertices[indexRead];
                function(index / 3, {A, B, C});
                
                A = B;
                B = C;
            }
        });
    }
}

void VROGeometryElement::processIndices(std::function<void (int, int)> function) const {
    VROByteBuffer buffer(_data->getData(), _data->getDataLength(), false);
    
    //TODO Support all primitive types!
    if (_primitiveType != VROGeometryPrimitiveType::Triangle) {
        return;
    }
    
    int indexCount = _primitiveCount * 3;
    
    for (int i = 0; i < indexCount; i++) {
        buffer.setPosition(i * _bytesPerIndex);
        int idx;
        if (_bytesPerIndex == 2) {
            idx = _signed ? buffer.readShort() : buffer.readUnsignedShort();
        }
        else if (_bytesPerIndex == 4) {
            idx = buffer.readInt();
        }
        else {
            pabort("Invalid bytes per index %d", _bytesPerIndex);
        }
        
        function(i, idx);
    }
}

