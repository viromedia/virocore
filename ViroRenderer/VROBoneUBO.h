//
//  VROBoneUBO.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/17/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROBoneUBO_h
#define VROBoneUBO_h

#include "VROOpenGL.h"
#include <vector>
#include <memory>

// Keep in sync with ViroFBX::VROFBXExporter.h and skinning_vsh.glsl
static const int kMaxBones = 192;
static const int kFloatsPerBone = 8;

// Grouped in 4N slots, matching skinning_vsh.glsl
typedef struct {
    int num_bones;
    float padding0, padding1, padding2;
    float bone_transforms[kMaxBones * kFloatsPerBone];
} VROBonesData;

class VROShaderProgram;
class VRODriverOpenGL;
class VROSkinner;
class VROShaderModifier;

/*
 Bones transformation matrices are written into UBOs. This way we 
 get flexibility (we're able to write large amounts of matrix data
 to the GPU) and structure (we can access this data as mat4 objects
 in GLSL).
 
 Each geometry with a skinner will have an associated VROBoneUBO, 
 which it updates whenever any of the bone matrices are updated, 
 typically after an animation frame.
 
 See VROLightingUBO.h for a detailed description of how UBOs work.
 */
class VROBoneUBO {
    
public:
    
    static std::shared_ptr<VROShaderModifier> createSkinningShaderModifier();
    static void unbind(std::shared_ptr<VROShaderProgram> &program);
    
    VROBoneUBO(std::shared_ptr<VRODriverOpenGL> driver);
    virtual ~VROBoneUBO();
    
    /*
     Bind this bone UBO into the "bones" block for the given program.
     */
    void bind(std::shared_ptr<VROShaderProgram> &program);
    
    /*
     Update the data in this UBO with the latest transformation 
     matrices in the provided skinner.
     */
    void update(const std::unique_ptr<VROSkinner> &skinner);
    
private:
    
    /*
     The uniform buffer object ID and binding point for bone parameters.
     */
    GLuint _bonesUBO;
    int _bonesUBOBindingPoint = 0;
    
    /*
     The driver that created this UBO.
     */
    std::weak_ptr<VRODriverOpenGL> _driver;
    
};

#endif /* VROBoneUBO_h */
