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

void VROGeometrySource::processVertices(std::function<void (int, VROVector4f)> function) const {
    VROByteBuffer buffer(_data->getData(), _data->getDataLength(), false);
    
    for (int i = 0; i < _vertexCount; i++) {
        buffer.setPosition(i * _dataStride + _dataOffset);
        
        float x = 0, y = 0, z = 0, w = 0;
        
        if (_floatComponents) {
            if (_bytesPerComponent == 2) {
                if (_componentsPerVertex > 0) {
                    x = buffer.readHalf();
                    if (_componentsPerVertex > 1) {
                        y = buffer.readHalf();
                        if (_componentsPerVertex > 2) {
                            z = buffer.readHalf();
                            if (_componentsPerVertex > 3) {
                                w = buffer.readHalf();
                            }
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
                            if (_componentsPerVertex > 3) {
                                w = buffer.readFloat();
                            }
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
                            if (_componentsPerVertex > 3) {
                                w = buffer.readByte();
                            }
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
                            if (_componentsPerVertex > 3) {
                                w = buffer.readShort();
                            }
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
                            if (_componentsPerVertex > 3) {
                                w = buffer.readInt();
                            }
                        }
                    }
                }
            }
            else {
                pabort("Invalid bytes per integer component %d", _componentsPerVertex);
            }
        }
        
        function(i, { x, y, z, w});
    }
}

void VROGeometrySource::modifyVertices(std::function<VROVector3f(int index, VROVector3f vertex)> function) const {
    VROByteBuffer buffer(_data->getData(), _data->getDataLength(), false);
    
    for (int i = 0; i < _vertexCount; i++) {
        buffer.setPosition(i * _dataStride + _dataOffset);
        
        float x = 0, y = 0, z = 0, w = 0;
        
        if (_floatComponents) {
            if (_bytesPerComponent == 2) {
                if (_componentsPerVertex > 0) {
                    x = buffer.readHalf();
                    if (_componentsPerVertex > 1) {
                        y = buffer.readHalf();
                        if (_componentsPerVertex > 2) {
                            z = buffer.readHalf();
                            if (_componentsPerVertex > 3) {
                                w = buffer.readHalf();
                            }
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
                            if (_componentsPerVertex > 3) {
                                w = buffer.readFloat();
                            }
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
                            if (_componentsPerVertex > 3) {
                                w = buffer.readByte();
                            }
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
                            if (_componentsPerVertex > 3) {
                                w = buffer.readShort();
                            }
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
                            if (_componentsPerVertex > 3) {
                                w = buffer.readInt();
                            }
                        }
                    }
                }
            }
            else {
                pabort("Invalid bytes per integer component %d", _componentsPerVertex);
            }
        }
        
        VROVector3f result = function(i, { x, y, z});
        
        buffer.setPosition(i * _dataStride + _dataOffset);
        if (_floatComponents) {
            if (_bytesPerComponent == 2) {
                if (_componentsPerVertex > 0) {
                    buffer.writeHalf(result.x);
                    if (_componentsPerVertex > 1) {
                        buffer.writeHalf(result.y);
                        if (_componentsPerVertex > 2) {
                            buffer.writeHalf(result.z);
                            if (_componentsPerVertex > 3) {
                                buffer.writeHalf(w);
                            }
                        }
                    }
                }
            }
            else if (_bytesPerComponent == 4) {
                if (_componentsPerVertex > 0) {
                    buffer.writeFloat(result.x);
                    if (_componentsPerVertex > 1) {
                        buffer.writeFloat(result.y);
                        if (_componentsPerVertex > 2) {
                            buffer.writeFloat(result.z);
                            if (_componentsPerVertex > 3) {
                                buffer.writeFloat(w);
                            }
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
                    buffer.writeByte(result.x);
                    if (_componentsPerVertex > 1) {
                        buffer.writeByte(result.y);
                        if (_componentsPerVertex > 2) {
                            buffer.writeByte(result.z);
                            if (_componentsPerVertex > 3) {
                                buffer.writeByte(w);
                            }
                        }
                    }
                }
            }
            else if (_bytesPerComponent == 2) {
                if (_componentsPerVertex > 0) {
                    buffer.writeShort(result.x);
                    if (_componentsPerVertex > 1) {
                        buffer.writeShort(result.y);
                        if (_componentsPerVertex > 2) {
                            buffer.writeShort(result.z);
                            if (_componentsPerVertex > 3) {
                                buffer.writeShort(w);
                            }
                        }
                    }
                }
            }
            else if (_bytesPerComponent == 4) {
                if (_componentsPerVertex > 0) {
                    buffer.writeInt(result.x);
                    if (_componentsPerVertex > 1) {
                        buffer.writeInt(result.y);
                        if (_componentsPerVertex > 2) {
                            buffer.writeInt(result.z);
                            if (_componentsPerVertex > 3) {
                                buffer.writeInt(w);
                            }
                        }
                    }
                }
            }
            else {
                pabort("Invalid bytes per integer component %d", _componentsPerVertex);
            }
        }
    }
}

VROBoundingBox VROGeometrySource::getBoundingBox() const {
    float minX =  FLT_MAX;
    float maxX = -FLT_MAX;
    float minY =  FLT_MAX;
    float maxY = -FLT_MAX;
    float minZ =  FLT_MAX;
    float maxZ = -FLT_MAX;
    
    processVertices([&minX, &maxX, &minY, &maxY, &minZ, &maxZ](int index, VROVector4f vertex) {
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
