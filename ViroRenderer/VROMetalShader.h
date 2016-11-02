//
//  VROMetalShader.h
//  ViroRenderer
//
//  Created by Raj Advani on 6/17/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROMetalShader_h
#define VROMetalShader_h

#include "VRODefines.h"
#if VRO_METAL

static std::atomic_int sShaderId;

class VROMetalShader {
public:
    VROMetalShader(id <MTLFunction> vertex, id <MTLFunction> fragment) :
        _shaderId(sShaderId++),
        _vertexProgram(vertex),
        _fragmentProgram(fragment) {
    }
    
    uint32_t getShaderId() const { return _shaderId; }
    id <MTLFunction> getVertexProgram() const { return _vertexProgram; }
    id <MTLFunction> getFragmentProgram() const { return _fragmentProgram; }
    
private:
    uint32_t _shaderId;
    id <MTLFunction> _vertexProgram;
    id <MTLFunction> _fragmentProgram;
};

#endif
#endif /* VROMetalShader_h */
