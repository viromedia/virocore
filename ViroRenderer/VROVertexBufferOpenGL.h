//
//  VROVertexBufferOpenGL.h
//  ViroKit
//
//  Created by Raj Advani on 6/29/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#ifndef VROVertexBufferOpenGL_h
#define VROVertexBufferOpenGL_h

#include <stdio.h>
#include <memory>
#include "VROOpenGL.h"
#include "VROVertexBuffer.h"

class VRODriverOpenGL;

class VROVertexBufferOpenGL : public VROVertexBuffer {
public:
    
    VROVertexBufferOpenGL(std::shared_ptr<VROData> data, std::shared_ptr<VRODriverOpenGL> driver);
    virtual ~VROVertexBufferOpenGL();
    
    virtual void hydrate();
    GLuint getVBO() const { return _buffer; }
    
private:
    
    GLuint _buffer;
    std::weak_ptr<VRODriverOpenGL> _driver;
    
};

#endif /* VROVertexBufferOpenGL_h */
