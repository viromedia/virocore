//
//  OBJLoaderDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_OBJLOADERDELEGATE_JNI_H
#define ANDROID_OBJLOADERDELEGATE_JNI_H

#include "VRODefines.h"
#include VRO_C_INCLUDE

enum class ModelType {
    Unknown = -1,
    OBJ = 1,
    FBX = 2,
    GLTF = 3,
    GLB = 4,
};
ModelType VROGetModelType(VRO_INT jModelType);

class VRONode;
class VROMaterial;
class OBJLoaderDelegate {
public:
    OBJLoaderDelegate(VRO_OBJECT nodeJavaObject, VRO_ENV env);
    ~OBJLoaderDelegate();

    void objLoaded(std::shared_ptr<VRONode> node, ModelType type, VRO_LONG requestId);
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
