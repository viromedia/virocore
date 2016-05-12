//
//  VROMaterialSubstrateOpenGL.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROMaterialSubstrateOpenGL_h
#define VROMaterialSubstrateOpenGL_h

#include "VROMaterial.h"
#include "VROMaterialSubstrate.h"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES3/glext.h>

class VRODriverOpenGL;

class VROMaterialSubstrateOpenGL : public VROMaterialSubstrate {
    
public:
    
    VROMaterialSubstrateOpenGL(const VROMaterial &material, const VRODriverOpenGL &driver);
    virtual ~VROMaterialSubstrateOpenGL();
    
private:
    
    const VROMaterial &_material;
    VROLightingModel _lightingModel;
    
};

#endif /* VROMaterialSubstrateOpenGL_h */
