//
//  VROParticleUBO.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <vector>
#include "VROParticle.h"
#include "VROParticleUBO.h"
#include "VROMath.h"
#include "VROShaderProgram.h"
#include "VROShaderModifier.h"
#include "VRODriverOpenGL.h"
#include "VROParticleEmitter.h"

int VROParticleUBO::sInstances = 0;
GLuint VROParticleUBO::sUBOVertexBufferID = 0;
GLuint VROParticleUBO::sUBOFragmentBufferID = 0;

VROParticleUBO::VROParticleUBO(std::shared_ptr<VRODriver> driver) {
    _driver = driver;

    sInstances++;
    if (sInstances > 1) {
        return;
    }

    // Set up Vertex UBO;
    glGenBuffers(1, &sUBOVertexBufferID);
    glBindBuffer(GL_UNIFORM_BUFFER, sUBOVertexBufferID);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VROParticlesUBOVertexData), NULL, GL_DYNAMIC_DRAW);

    // Set up Fragment UBO;
    glGenBuffers(1, &sUBOFragmentBufferID);
    glBindBuffer(GL_UNIFORM_BUFFER, sUBOFragmentBufferID);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VROParticlesUBOFragmentData), NULL, GL_DYNAMIC_DRAW);
    _lastKnownBoundingBox = VROBoundingBox(0,0,0,0,0,0);
}

VROParticleUBO::~VROParticleUBO() {
    sInstances --;
    if (sInstances > 1) {
        return;
    }

    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (driver) {
        glDeleteBuffers(1, &sUBOVertexBufferID);
        glDeleteBuffers(1, &sUBOFragmentBufferID);
    }
}

std::vector<std::shared_ptr<VROShaderModifier>>  VROParticleUBO::createInstanceShaderModifier() {
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers;
    std::vector<std::string> vertexModifierCode =  {
            "_transforms.model_matrix = particles_vertex_transform[v_instance_id];",
    };

    // Note: A surface modifier is used here due to a bug in adding shader modifier code:
    // This is because bloom effects are applied before this code is run, resulting in
    // undesired effects. To get around that, we apply a surface modifier instead.
    std::vector<std::string> surfaceModifierCode = {
            "highp vec4 particleColorAll = particles_fragment_color[v_instance_id];",
            "highp vec3 particleJustColor = vec3(particleColorAll.x, particleColorAll.y, particleColorAll.z);",
            "highp float particleAlpha = particleColorAll.w;",
            "_surface.alpha = _surface.alpha * particleAlpha;",
            "_surface.diffuse_color.xyz = _surface.diffuse_color.xyz * particleJustColor;"
    };

    modifiers.push_back(
            std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry,  vertexModifierCode));
    modifiers.push_back(
            std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface, surfaceModifierCode));
    return modifiers;
}

void VROParticleUBO::bind() {
    glBindBufferBase(GL_UNIFORM_BUFFER, VROShaderProgram::sParticleVertexUBOBindingPoint, sUBOVertexBufferID);
    glBindBufferBase(GL_UNIFORM_BUFFER, VROShaderProgram::sParticleFragmentUBOBindingPoint, sUBOFragmentBufferID);
}

int VROParticleUBO::getNumberOfDrawCalls() {
    if (_lastKnownParticles.size() == 0) {
        return -1;
    }

    float totalParticles = _lastKnownParticles.size();
    float maxParticlesPerUBO = kMaxParticlesPerUBO;
    float requiredNumberOfDraws = ceil(totalParticles / maxParticlesPerUBO);
    return (int) requiredNumberOfDraws;
}

int VROParticleUBO::bindDrawData(int currentDrawCallIndex) {
    if (_lastKnownParticles.size() == 0) {
        return 0;
    }

    // Grab the window of particles that corresponds to this currentDrawCallIndex
    int start = currentDrawCallIndex * kMaxParticlesPerUBO;
    int end = (currentDrawCallIndex + 1) * kMaxParticlesPerUBO;
    if (_lastKnownParticles.size() < end ) {
        end = (int) _lastKnownParticles.size() - 1;
    }

    // Parse / serialize the data into the uniform buffer object
    VROParticlesUBOVertexData vertexData;
    VROParticlesUBOFragmentData fragmentData;
    for (int i = start; i < end; i++) {
        const float *transformArray = _lastKnownParticles[i].currentWorldTransform.getArray();
        memcpy(&vertexData.particles_transform[(i - start) * kMaxFloatsPerTransform],
               transformArray,
               kMaxFloatsPerTransform * sizeof(float));

        fragmentData.frag_particles_color[(i - start) * 4 + 0] = _lastKnownParticles[i].colorCurrent.x;
        fragmentData.frag_particles_color[(i - start) * 4 + 1] = _lastKnownParticles[i].colorCurrent.y;
        fragmentData.frag_particles_color[(i - start) * 4 + 2] = _lastKnownParticles[i].colorCurrent.z;
        fragmentData.frag_particles_color[(i - start) * 4 + 3] = _lastKnownParticles[i].colorCurrent.w;
    }

    // Finally bind the UBO to its corresponding buffers.
    pglpush("Particles");
    glBindBuffer(GL_UNIFORM_BUFFER, sUBOVertexBufferID);
#if VRO_AVOID_BUFFER_SUB_DATA
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VROParticlesUBOVertexData), &vertexData, GL_DYNAMIC_DRAW);
#else
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VROParticlesUBOVertexData), &vertexData);
#endif
    pglpop();

    pglpush("ParticlesFragment");
    glBindBuffer(GL_UNIFORM_BUFFER, sUBOFragmentBufferID);
#if VRO_AVOID_BUFFER_SUB_DATA
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VROParticlesUBOFragmentData), &fragmentData, GL_DYNAMIC_DRAW);
#else
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VROParticlesUBOFragmentData), &fragmentData);
#endif
    pglpop();
    return end - start;
}

void VROParticleUBO::update(std::vector<VROParticle> &particles, VROBoundingBox &particleBox) {
    _lastKnownParticles = particles;
    _lastKnownBoundingBox = particleBox;
}

VROBoundingBox VROParticleUBO::getInstancedBoundingBox(){
    return _lastKnownBoundingBox;
}
