//
//  VROBoneUBO.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/17/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

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

static std::shared_ptr<VROShaderModifier> sSkinningShaderModifier;
static std::shared_ptr<VROShaderModifier> sSkinningShaderModifierWithScale;

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
            }
            
            return sSkinningShaderModifierWithScale;
        }
        else {
            if (!sSkinningShaderModifier) {
                std::vector<std::string> modifierCode =  {
                    "mat2x4 blended_dq = get_blended_dual_quaternion(_geometry.bone_indices, _geometry.bone_weights);",
                    "_geometry.position = dual_quat_transform_point(_geometry.position.xyz, blended_dq[0], blended_dq[1]);",
                    "_geometry.normal = quat_rotate_vector(_geometry.normal.xyz, blended_dq[0]);"
                };
                sSkinningShaderModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry,
                                                                              modifierCode);
                sSkinningShaderModifierWithScale->setName("skindq");
            }
            
            return sSkinningShaderModifier;
        }
    }
    else {
        if (!sSkinningShaderModifier) {
            std::vector<std::string> modifierCode =  {
                    "vec4 pos_h = vec4(_geometry.position, 1.0);",
                    "vec4 pos_blended = (bone_matrices[_geometry.bone_indices.x] * pos_h) * _geometry.bone_weights.x + "
                                       "(bone_matrices[_geometry.bone_indices.y] * pos_h) * _geometry.bone_weights.y + "
                                       "(bone_matrices[_geometry.bone_indices.z] * pos_h) * _geometry.bone_weights.z + "
                                       "(bone_matrices[_geometry.bone_indices.w] * pos_h) * _geometry.bone_weights.w;",
                    "_geometry.position = pos_blended.xyz;"
            };
            sSkinningShaderModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry,
                                                                          modifierCode);
            sSkinningShaderModifierWithScale->setName("skin");
        }
        
        return sSkinningShaderModifier;
    }
}

VROBoneUBO::VROBoneUBO(std::shared_ptr<VRODriverOpenGL> driver) :
    _driver(driver) {
    
    _bonesUBOBindingPoint = driver->generateBindingPoint();
    
    glGenBuffers(1, &_bonesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, _bonesUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VROBonesData), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    // Links the UBO and the binding point
    glBindBufferBase(GL_UNIFORM_BUFFER, _bonesUBOBindingPoint, _bonesUBO);
}

VROBoneUBO::~VROBoneUBO() {
    std::shared_ptr<VRODriverOpenGL> driver = _driver.lock();
    if (driver) {
        driver->internBindingPoint(_bonesUBOBindingPoint);
        glDeleteBuffers(1, &_bonesUBO);
    }
}

void VROBoneUBO::unbind(std::shared_ptr<VROShaderProgram> &program) {
    if (program->hasBonesBlock()) {
        glUniformBlockBinding(program->getProgram(), program->getBonesBlockIndex(), 0);
    }
}

void VROBoneUBO::bind(std::shared_ptr<VROShaderProgram> &program) {
    if (program->hasBonesBlock()) {
        glUniformBlockBinding(program->getProgram(), program->getBonesBlockIndex(), _bonesUBOBindingPoint);
    }
}

void VROBoneUBO::update(const std::unique_ptr<VROSkinner> &skinner) {
    pglpush("Bones");
    
    VROBonesData data;
    int numBones = skinner->getSkeleton()->getNumBones();
    for (int i = 0; i < numBones; i++) {
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
            
            int floatsPerBone = kFloatsPerBone;
            data.bone_transforms[i * floatsPerBone + 0] = real.X;
            data.bone_transforms[i * floatsPerBone + 1] = real.Y;
            data.bone_transforms[i * floatsPerBone + 2] = real.Z;
            data.bone_transforms[i * floatsPerBone + 3] = real.W;
            data.bone_transforms[i * floatsPerBone + 4] = dual.X;
            data.bone_transforms[i * floatsPerBone + 5] = dual.Y;
            data.bone_transforms[i * floatsPerBone + 6] = dual.Z;
            data.bone_transforms[i * floatsPerBone + 7] = dual.W;
            data.bone_transforms[i * floatsPerBone + 8] = scale.x;
            data.bone_transforms[i * floatsPerBone + 9] = scale.y;
            data.bone_transforms[i * floatsPerBone + 10] = scale.z;
            data.bone_transforms[i * floatsPerBone + 11] = 1.0;
        }
        else {
            memcpy(&data.bone_transforms[i * kFloatsPerBone], transform.getArray(), kFloatsPerBone * sizeof(float));
        }
    }
    
    glBindBuffer(GL_UNIFORM_BUFFER, _bonesUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VROBonesData), &data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    pglpop();
}
