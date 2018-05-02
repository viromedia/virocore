//
//  TextDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_TEXTDELEGATE_JNI_H
#define ANDROID_TEXTDELEGATE_JNI_H

#include "VRODefines.h"
#include VRO_C_INCLUDE

class TextDelegate {
public:
    TextDelegate(VRO_OBJECT textJavaObject, VRO_ENV env);
    ~TextDelegate();

    void textCreated(VRO_REF(VROText) native_text_ref);

private:
    VRO_OBJECT _javaObject;

};
#endif //ANDROID_TEXTDELEGATE_JNI_H
