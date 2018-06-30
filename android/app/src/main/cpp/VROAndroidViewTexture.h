//
//  VROAndroidViewTexture.h
//  ViroRenderer
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROAndroidViewTexture_h
#define VROAndroidViewTexture_h

#include "VROTexture.h"
#include "VROOpenGL.h"
class VRORenderContext;
class VROFrameSynchronizer;
class VRODriver;
class VRODriverOpenGL;

/*
 VROAndroidViewTexture is responsible for constructing and initializing
 surfaces and glTextures required for rendering Android pixel data onto
 VROTextures for a given AndroidViewTexture.java object in a 3D scene.
 */
class VROAndroidViewTexture : public VROTexture {
public:
    /*
     Constructs the VROAndroidViewTexture. After construction, init() must be
     asynchronously called on the rendering thread (from the UI thread).
     */
    VROAndroidViewTexture(VRO_OBJECT jRenderableTexture, int width, int height);
    ~VROAndroidViewTexture();

    /*
     Must be invoked from the rendering thread after construction.
     */
    void init(std::shared_ptr<VRODriverOpenGL> driver);

private:
    /*
     The java object associated with this VROAndroidViewTexture that contains a reference
     to the Android views to be rendered.
     */
    jobject _jAndroidViewTexture;

    /*
     The VideoSink object used by the renderer to receive canvas pixel data to be rendered
     in the scene. This acts as the 'receiver' of pixel data provided by jAndroidViewTexture.
     */
    jobject _jVideoSinkSurface;

    /*
     The texture ID representing the underlying OpenGL texture for this VROAndroidViewTexture.
     */
    GLuint _textureId;
};

#endif /* VROAndroidViewTexture */
