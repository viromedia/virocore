//
//  OBJLoaderDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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

    /*
     If the model is removed before load completes, this delegate (which is held by the
     finishCallback, which in turn is held by hydrationCallbacks within the VRONode) will
     only be cleaned up if the source Object3D is cleaned up; therefore it must hold a weak
     pointer to OBJ. Otherwise we will have a strong reference cycle between Object3D and resources
     used by VRONode.
     */
    VRO_WEAK _javaObject;

    /*
     Creates a map of unique jMaterials for a given VRONode, recursively.
     */
    static void generateJMaterials(std::map<std::string, std::shared_ptr<VROMaterial>> &mats,
                                   std::shared_ptr<VRONode> node);
};


#endif //ANDROID_OBJLOADERDELEGATE_JNI_H
