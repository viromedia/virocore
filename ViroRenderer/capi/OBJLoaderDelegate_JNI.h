//
//  OBJLoaderDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_OBJLOADERDELEGATE_JNI_H
#define ANDROID_OBJLOADERDELEGATE_JNI_H

#include <PersistentRef.h>

#include "VRODefines.h"
#include VRO_C_INCLUDE

class VRONode;
class VROMaterial;
class OBJLoaderDelegate {
public:
    OBJLoaderDelegate(VRO_OBJECT nodeJavaObject, JNIEnv *env);
    ~OBJLoaderDelegate();

    void objLoaded(std::shared_ptr<VRONode> node, bool isFBX, jlong requestId);
    void objFailed(std::string error);
private:
    VRO_OBJECT _javaObject;

    /*
     Creates a map of unique jMaterials for a given VRONode, recursively.
     */
    static void generateJMaterials(std::map<std::string, std::shared_ptr<VROMaterial>> &mats,
                                   std::shared_ptr<VRONode> node);
};


#endif //ANDROID_OBJLOADERDELEGATE_JNI_H
