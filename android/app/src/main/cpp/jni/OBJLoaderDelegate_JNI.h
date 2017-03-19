//
//  OBJLoaderDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_OBJLOADERDELEGATE_JNI_H
#define ANDROID_OBJLOADERDELEGATE_JNI_H

#include <jni.h>
#include <PersistentRef.h>

class OBJLoaderDelegate {
public:
    OBJLoaderDelegate(jobject nodeJavaObject, JNIEnv *env);
    ~OBJLoaderDelegate();

    void objLoaded();
    void objFailed(std::string error);

private:
    jobject _javaObject;

};


#endif //ANDROID_OBJLOADERDELEGATE_JNI_H
