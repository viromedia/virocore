//
//  VROGeometrySource.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROGeometrySource.h"
#include "VROBoundingBox.h"
#include "VROByteBuffer.h"
#include "VROMath.h"
#include "VROLog.h"
#include <float.h>

void VROGeometrySource::processVertices(std::function<void (int, VROVector3f)> function) const {
    VROByteBuffer buffer(_data->getData(), _data->getDataLength(), false);
    
    for (int i = 0; i < _vertexCount; i++) {
        buffer.setPosition(i * _dataStride + _dataOffset);
        
        float x, y, z;
        
        if (_floatComponents) {
            if (_bytesPerComponent == 2) {
                if (_componentsPerVertex > 0) {
                    x = buffer.readHalf();
                    if (_componentsPerVertex > 1) {
                        y = buffer.readHalf();
                        if (_componentsPerVertex > 2) {
                            z = buffer.readHalf();
                        }
                    }
                }
            }
            else if (_bytesPerComponent == 4) {
                if (_componentsPerVertex > 0) {
                    x = buffer.readFloat();
                    if (_componentsPerVertex > 1) {
                        y = buffer.readFloat();
                        if (_componentsPerVertex > 2) {
                            z = buffer.readFloat();
                        }
                    }
                }
            }
            else {
                pabort("Invalid bytes per floating point component %d", _componentsPerVertex);
            }
        }
        else {
            if (_bytesPerComponent == 1) {
                if (_componentsPerVertex > 0) {
                    x = buffer.readByte();
                    if (_componentsPerVertex > 1) {
                        y = buffer.readByte();
                        if (_componentsPerVertex > 2) {
                            z = buffer.readByte();
                        }
                    }
                }
            }
            else if (_bytesPerComponent == 2) {
                if (_componentsPerVertex > 0) {
                    x = buffer.readShort();
                    if (_componentsPerVertex > 1) {
                        y = buffer.readShort();
                        if (_componentsPerVertex > 2) {
                            z = buffer.readShort();
                        }
                    }
                }
            }
            else if (_bytesPerComponent == 4) {
                if (_componentsPerVertex > 0) {
                    x = buffer.readInt();
                    if (_componentsPerVertex > 1) {
                        y = buffer.readInt();
                        if (_componentsPerVertex > 2) {
                            z = buffer.readInt();
                        }
                    }
                }
            }
            else {
                pabort("Invalid bytes per integer component %d", _componentsPerVertex);
            }
        }
        
        function(i, { x, y, z});
    }
}

VROBoundingBox VROGeometrySource::getBoundingBox() const {
    float minX =  FLT_MAX;
    float maxX = -FLT_MAX;
    float minY =  FLT_MAX;
    float maxY = -FLT_MAX;
    float minZ =  FLT_MAX;
    float maxZ = -FLT_MAX;
    
    processVertices([&minX, &maxX, &minY, &maxY, &minZ, &maxZ](int index, VROVector3f vertex) {
        if (vertex.x < minX) {
            minX = vertex.x;
        }
        if (vertex.x > maxX) {
            maxX = vertex.x;
        }
        if (vertex.y < minY) {
            minY = vertex.y;
        }
        if (vertex.y > maxY) {
            maxY = vertex.y;
        }
        if (vertex.z < minZ) {
            minZ = vertex.z;
        }
        if (vertex.z > maxZ) {
            maxZ = vertex.z;
        }
    });
    
    return VROBoundingBox(minX, maxX, minY, maxY, minZ, maxZ);
}