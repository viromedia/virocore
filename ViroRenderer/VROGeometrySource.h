//
//  VROGeometrySource.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROGeometrySource_h
#define VROGeometrySource_h

#include <stdio.h>
#include <memory>
#include "VROData.h"

enum class VROGeometrySourceSemantic {
    Vertex,
    Normal,
    Color,
    Texcoord,
    VertexCrease,
    EdgeCrease,
    BoneWeights,
    BoneIndices
};

/*
 Specifies per-vertex data for the set of vertices forming the surface of a three-dimensional 
 object, or geometry. Used together with VROGeometryElement objects to define custom VROGeometry objects
 or to inspect the data that composes an existing geometry.
 */
class VROGeometrySource {
    
public:
    
    VROGeometrySource(std::shared_ptr<VROData> data,
                      VROGeometrySourceSemantic semantic,
                      int vertexCount,
                      int componentsPerVertex,
                      int bytesPerComponent,
                      int dataOffset,
                      int dataStride) :
        _data(data),
        _semantic(semantic),
        _vertexCount(vertexCount),
        _componentsPerVertex(componentsPerVertex),
        _bytesPerComponent(bytesPerComponent),
        _dataOffset(dataOffset),
        _dataStride(dataStride)
    {}
    
private:
    
    /*
     The interleaved raw vertex data.
     */
    std::shared_ptr<VROData> _data;
    
    /*
     The attribute the geometry source is describing for each vertex.
     */
    VROGeometrySourceSemantic _semantic;
    
    /*
     The number of vertices described in the _data array.
     */
    int _vertexCount;
    
    /*
     The number of scalar components per vertex (i.e. x, y, z for position).
     */
    int _componentsPerVertex;
    
    /*
     The number of bytes per scalar component (i.e. 4 for floats).
     */
    int _bytesPerComponent;
    
    /*
     The offset in the data array at which point this source's data begins. Used
     for interleaving a vertex array.
     */
    int _dataOffset;
    
    /*
     The number of bytes from one vertex in the data to the next.
     */
    int _dataStride;
    
};

#endif /* VROGeometrySource_h */
