//
//  VROBoneUBO.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/17/17.
//  Copyright © 2017 Viro Media. All rights reserved.
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

#include "VROBoneUBO.h"
#include "VROBone.h"
#include "VROLog.h"
#include "VROMath.h"
#include "VROSkinner.h"
#include "VROSkeleton.h"
#include "VROShaderProgram.h"
#include "VROShaderModifier.h"
#include "VRODriverOpenGL.h"
#include "VRODualQuaternion.h"

static thread_local std::shared_ptr<VROShaderModifier> sSkinningShaderModifier;
static thread_local std::shared_ptr<VROShaderModifier> sSkinningShaderModifierWithScale;

std::shared_ptr<VROShaderModifier> VROBoneUBO::createSkinningShaderModifier(bool hasScale) {
    if (kDualQuaternionEnabled) {
        /*
         Modifier that performs skeletal animation in the vertex shader. Uses dual-quaternion
         skinning, with functions provided in skinning_vsh.glsl. We have an optimized path if
         there are no scale transforms.
         */
        if (hasScale) {
            if (!sSkinningShaderModifierWithScale) {
                std::vector<std::string> modifierCode =  {
                    "#include skinning_vsh",
                    "vec4 blended_s = get_blended_scale(_geometry.bone_indices, _geometry.bone_weights);",
                    "_geometry.position = _geometry.position * blended_s.xyz;",
                    "_geometry.normal = _geometry.normal / blended_s.xyz;", // Equivalent to multiplying by inverse-transpose of scale matrix
                    
                    "mat2x4 blended_dq = get_blended_dual_quaternion(_geometry.bone_indices, _geometry.bone_weights);",
                    "_geometry.position = dual_quat_transform_point(_geometry.position.xyz, blended_dq[0], blended_dq[1]);",
                    "_geometry.normal = quat_rotate_vector(_geometry.normal.xyz, blended_dq[0]);"
                };
                sSkinningShaderModifierWithScale = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry,
                                                                                       modifierCode);
                sSkinningShaderModifierWithScale->setName("skindqs");
                sSkinningShaderModifierWithScale->setAttributes((int) VROShaderMask::BoneIndex | (int) VROShaderMask::BoneWeight);
            }
            
            return sSkinningShaderModifierWithScale;
        }
        else {
            if (!sSkinningShaderModifier) {
                std::vector<std::string> modifierCode =  {
                    "#include skinning_vsh",
                    "mat2x4 blended_dq = get_blended_dual_quaternion(_geometry.bone_indices, _geometry.bone_weights);",
                    "_geometry.position = dual_quat_transform_point(_geometry.position.xyz, blended_dq[0], blended_dq[1]);",
                    "_geometry.normal = quat_rotate_vector(_geometry.normal.xyz, blended_dq[0]);"
                };
                sSkinningShaderModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry,
                                                                              modifierCode);
                sSkinningShaderModifier->setName("skindq");
                sSkinningShaderModifier->setAttributes((int) VROShaderMask::BoneIndex | (int) VROShaderMask::BoneWeight);
            }
            
            return sSkinningShaderModifier;
        }
    }
    else {
        if (!sSkinningShaderModifier) {
            std::vector<std::string> modifierCode =  {
                    "#include skinning_vsh",
                    "vec4 pos_h = vec4(_geometry.position, 1.0);",
                    "vec4 pos_blended = (bone_matrices[_geometry.bone_indices.x] * pos_h) * _geometry.bone_weights.x + "
                                       "(bone_matrices[_geometry.bone_indices.y] * pos_h) * _geometry.bone_weights.y + "
                                       "(bone_matrices[_geometry.bone_indices.z] * pos_h) * _geometry.bone_weights.z + "
                                       "(bone_matrices[_geometry.bone_indices.w] * pos_h) * _geometry.bone_weights.w;",
                    "_geometry.position = pos_blended.xyz;"
            };
            sSkinningShaderModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry,
                                                                          modifierCode);
            sSkinningShaderModifier->setName("skin");
            sSkinningShaderModifier->setAttributes((int) VROShaderMask::BoneIndex | (int) VROShaderMask::BoneWeight);
        }
        
        return sSkinningShaderModifier;
    }
}

VROBoneUBO::VROBoneUBO(std::shared_ptr<VRODriverOpenGL> driver) :
    _driver(driver) {
    
    GL( glGenBuffers(1, &_bonesUBO) );
    GL( glBindBuffer(GL_UNIFORM_BUFFER, _bonesUBO) );

    // If we don't initialize the bone data, the GPU may freeze (in particular when using
    // Adreno + OVR)
    VROBonesData data;
    VROMatrix4f identity;
    for (int i = 0, iFPB = 0; i < kMaxBones; i++, iFPB += kFloatsPerBone) {
        memcpy(&data.bone_transforms[iFPB], identity.getArray(), kFloatsPerBone * sizeof(float));
    }
    GL( glBindBuffer(GL_UNIFORM_BUFFER, _bonesUBO) );
    GL( glBufferData(GL_UNIFORM_BUFFER, sizeof(VROBonesData), &data, GL_DYNAMIC_DRAW) );
}

VROBoneUBO::~VROBoneUBO() {
    std::shared_ptr<VRODriverOpenGL> driver = _driver.lock();
    if (driver) {
        driver->deleteBuffer(_bonesUBO);
    }
}

void VROBoneUBO::bind() {
    GL( glBindBufferBase(GL_UNIFORM_BUFFER, VROShaderProgram::sBonesUBOBindingPoint, _bonesUBO) );
}

void VROBoneUBO::update(const std::shared_ptr<VROSkinner> &skinner) {
    pglpush("Bones");
    
    VROBonesData data;
    int numBones = skinner->getSkeleton()->getNumBones();
    for (int i = 0, iFPB = 0; i < numBones; i++, iFPB += kFloatsPerBone) {
        if (i >= kMaxBones) {
            break;
        }

        VROMatrix4f transform = skinner->getModelTransform(i);
        if (kDualQuaternionEnabled) {
            VROVector3f   translation = transform.extractTranslation();
            VROVector3f   scale = transform.extractScale();
            VROQuaternion rotation = transform.extractRotation(scale);
            
            /*
             Convert the rotation and translation to a dual quaternion. The scale
             is included separately. Load all into the UBO.
             */
            VRODualQuaternion dq(translation, rotation);
            VROQuaternion real = dq.getReal();
            VROQuaternion dual = dq.getDual();
            
            data.bone_transforms[iFPB + 0] = real.X;
            data.bone_transforms[iFPB + 1] = real.Y;
            data.bone_transforms[iFPB + 2] = real.Z;
            data.bone_transforms[iFPB + 3] = real.W;
            data.bone_transforms[iFPB + 4] = dual.X;
            data.bone_transforms[iFPB + 5] = dual.Y;
            data.bone_transforms[iFPB + 6] = dual.Z;
            data.bone_transforms[iFPB + 7] = dual.W;
            data.bone_transforms[iFPB + 8] = scale.x;
            data.bone_transforms[iFPB + 9] = scale.y;
            data.bone_transforms[iFPB + 10] = scale.z;
            data.bone_transforms[iFPB + 11] = 1.0;
        }
        else {
            memcpy(&data.bone_transforms[iFPB], transform.getArray(), kFloatsPerBone * sizeof(float));
        }
    }
    
    GL( glBindBuffer(GL_UNIFORM_BUFFER, _bonesUBO) );
#if VRO_AVOID_BUFFER_SUB_DATA
    GL( glBufferData(GL_UNIFORM_BUFFER, sizeof(VROBonesData), &data, GL_DYNAMIC_DRAW) );
#else
    GL( glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VROBonesData), &data) );
#endif
    
    pglpop();
}
