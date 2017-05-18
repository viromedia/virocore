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
#include "VRODriverOpenGL.h"

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
    data.num_bones = 0;
    
    int numBones = skinner->getSkeleton()->getNumBones();
    for (int i = 0; i < numBones; i++) {
        if (i >= kMaxBones) {
            break;
        }
        
        VROMatrix4f transform = skinner->getModelTransform(i);
        memcpy(&data.bone_matrices[i * kFloatsPerBone], transform.getArray(), kFloatsPerBone * sizeof(float));
        data.num_bones++;
    }
    
    glBindBuffer(GL_UNIFORM_BUFFER, _bonesUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VROBonesData), &data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    pglpop();
}
