//
//  TextDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_TEXTDELEGATE_JNI_H
#define ANDROID_TEXTDELEGATE_JNI_H

#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

class TextDelegate {
public:
    TextDelegate(VRO_OBJECT textJavaObject, VRO_ENV env);
    ~TextDelegate();

    static VRO_REF(TextDelegate) jptr(std::shared_ptr<TextDelegate> shared_node) {
        PersistentRef<TextDelegate> *native_text = new PersistentRef<TextDelegate>(shared_node);
        return reinterpret_cast<intptr_t>(native_text);
    }

    static std::shared_ptr<TextDelegate> native(VRO_REF(TextDelegate) ptr) {
        PersistentRef<TextDelegate> *persistentDelegate = reinterpret_cast<PersistentRef<TextDelegate> *>(ptr);
        return persistentDelegate->get();
    }

    void textCreated(VRO_REF(VROText) native_text_ref);

private:
    VRO_OBJECT _javaObject;

};
#endif //ANDROID_TEXTDELEGATE_JNI_H
