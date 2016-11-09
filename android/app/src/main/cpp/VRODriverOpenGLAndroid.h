//
//  VRODriverOpenGLAndroid.h
//  ViroKit
//
//  Created by Raj Advani on 11/8/16.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef ANDROID_VRODRIVEROPENGLANDROID_H
#define ANDROID_VRODRIVEROPENGLANDROID_H

#include "VRODriverOpenGL.h"

class VRODriverOpenGLAndroid : public VRODriverOpenGL {

public:

    VRODriverOpenGLAndroid() {
    }
    virtual ~VRODriverOpenGLAndroid() { }

    VROVideoTextureCache *newVideoTextureCache() {
        // TODO Android
        return nullptr;
    }

private:

};

#endif //ANDROID_VRODRIVEROPENGLANDROID_H
