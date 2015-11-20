//
//  VROGeometryElement.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROGeometryElement_h
#define VROGeometryElement_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include "VROData.h"

enum class VROGeometryPrimitiveType {
    Triangle,
    TriangleStrip,
    Line,
    Point
};

/*
 Describes how vertices are connected to form the surface of a three-dimensional object, or geometry.
 Used in conjunction with VROGeometrySource.
 */
class VROGeometryElement {
    
public:
    
    VROGeometryElement(std::shared_ptr<VROData> data, VROGeometryPrimitiveType primitiveType,
                       int primitiveCount, int bytesPerIndex) :
        _primitiveType(primitiveType),
        _primitiveCount(primitiveCount),
        _data(data),
        _bytesPerIndex(bytesPerIndex)
    {}
    
    std::shared_ptr<VROData> getData() const {
        return _data;
    }
    VROGeometryPrimitiveType getPrimitiveType() const {
        return _primitiveType;
    }
    int getPrimitiveCount() const {
        return _primitiveCount;
    }
    int getBytesPerIndex() const {
        return _bytesPerIndex;
    }
    
private:
    
    /*
     The type of the primitives we should create from the associated geometry
     source using the indices in this element.
     */
    VROGeometryPrimitiveType _primitiveType;
    
    /*
     The number of triangles, triangle strips, etc.
     */
    const int _primitiveCount;
    
    /*
     The index data, and the size of each index.
     */
    std::shared_ptr<VROData> _data;
    const int _bytesPerIndex;
    
};

#endif /* VROGeometryElement_h */
