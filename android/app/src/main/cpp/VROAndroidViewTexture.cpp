//
//  VROAndroidViewTexture.h
//  ViroRenderer
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include <capi/ViroContext_JNI.h>
#include "VROAndroidViewTexture.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VROPlatformUtil.h"
#include "VRODriverOpenGL.h"

#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_AndroidViewTexture_##method_name

extern "C" {
VRO_METHOD(VRO_REF(VROAndroidViewTexture), nativeCreateAndroidViewTexture)(VRO_ARGS
                                                                          VRO_REF(ViroContext) context_j,
                                                                          VRO_INT width,
                                                                          VRO_INT height) {
    VRO_METHOD_PREAMBLE;
    std::weak_ptr<ViroContext> context_w = VRO_REF_GET(ViroContext, context_j);
    std::shared_ptr<VROAndroidViewTexture> androidViewTexture
            = std::make_shared<VROAndroidViewTexture>(obj, width, height);

    VROPlatformDispatchAsyncRenderer([androidViewTexture, context_w] {
        std::shared_ptr<ViroContext> context = context_w.lock();
        if (!context) {
            return;
        }

        androidViewTexture->init(std::dynamic_pointer_cast<VRODriverOpenGL>(context->getDriver()));
    });

    return VRO_REF_NEW(VROAndroidViewTexture, androidViewTexture);
}

VRO_METHOD(void, nativeDeleteAndroidViewTexture)(VRO_ARGS
                                                VRO_REF(VROAndroidViewTexture) textureRef) {
    VRO_REF_DELETE(VROAndroidViewTexture, textureRef);
}

} // extern "C"

VROAndroidViewTexture::VROAndroidViewTexture(VRO_OBJECT jRenderableTexture, int width, int height):
        VROTexture(VROTextureType::TextureEGLImage, VROTextureInternalFormat::RGBA8, VROStereoMode::None),
        _textureId(0) {

    JNIEnv *env = VROPlatformGetJNIEnv();
    _jAndroidViewTexture = env->NewGlobalRef(jRenderableTexture);
    _jVideoSinkSurface = NULL;
    _height = height;
    _width = width;
}

VROAndroidViewTexture::~VROAndroidViewTexture() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->DeleteGlobalRef(_jAndroidViewTexture);
    env->DeleteGlobalRef(_jVideoSinkSurface);
    VROPlatformDestroyVideoSink(_textureId);
    _jAndroidViewTexture = NULL;
    _jVideoSinkSurface = NULL;
}

void VROAndroidViewTexture::init(std::shared_ptr<VRODriverOpenGL> driver) {
    // Generate the GL Texture on which to render Android Views.
    glGenTextures(1, &_textureId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, _textureId);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Associated the substrate with the generated Texture
    std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateOpenGL>(
            new VROTextureSubstrateOpenGL(GL_TEXTURE_EXTERNAL_OES, _textureId, driver, true));
    setSubstrate(0, std::move(substrate));

    // Finally create the VideoSink's surface to be used for rendering our Android views onto.
    jobject jSurface = VROPlatformCreateVideoSink(_textureId, _width, _height);
    JNIEnv *env = VROPlatformGetJNIEnv();
    _jVideoSinkSurface = env->NewGlobalRef(jSurface);

    // Bind the VideoSink's surface to our RenderableAndroidTexture.java
    jclass cls = env->GetObjectClass(_jAndroidViewTexture);
    jmethodID jmethod = env->GetMethodID(cls, "setVideoSink", "(Landroid/view/Surface;)V");
    env->CallVoidMethod(_jAndroidViewTexture, jmethod, _jVideoSinkSurface);
    env->DeleteLocalRef(cls);
}