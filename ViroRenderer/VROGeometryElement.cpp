//
//  VROGeometryElement.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROGeometryElement.h"
#include "VROByteBuffer.h"
#include "VROGeometrySource.h"
#include "VROLog.h"

void VROGeometryElement::processTriangles(std::function<void(int index, VROTriangle triangle)> function,
                                          std::shared_ptr<VROGeometrySource> geometrySource) const {
    
    std::vector<VROVector3f> vertices;
    geometrySource->processVertices([&vertices](int index, VROVector3f vertex) {
        vertices.push_back(vertex);
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
    
    //TODO Make this class store index count instead!
    int indexCount = _primitiveCount * 3;
    
    for (int i = 0; i < indexCount; i++) {
        buffer.setPosition(i * _bytesPerIndex);
        int idx;
        if (_bytesPerIndex == 2) {
            idx = buffer.readShort();
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
