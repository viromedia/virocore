//
//  TextDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_TEXTDELEGATE_JNI_H
#define ANDROID_TEXTDELEGATE_JNI_H

#include <jni.h>
#include <PersistentRef.h>

class TextDelegate {
public:
    TextDelegate(jobject textJavaObject, JNIEnv *env);
    ~TextDelegate();

    static jlong jptr(std::shared_ptr<TextDelegate> shared_node) {
        PersistentRef<TextDelegate> *native_text = new PersistentRef<TextDelegate>(shared_node);
        return reinterpret_cast<intptr_t>(native_text);
    }

    static std::shared_ptr<TextDelegate> native(jlong ptr) {
        PersistentRef<TextDelegate> *persistentDelegate = reinterpret_cast<PersistentRef<TextDelegate> *>(ptr);
        return persistentDelegate->get();
    }

    void textCreated(jlong native_text_ref);

private:
    jobject _javaObject;

};
#endif //ANDROID_TEXTDELEGATE_JNI_H
