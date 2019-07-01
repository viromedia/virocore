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

VROParticleUBO::VROParticleUBO(std::shared_ptr<VRODriver> driver) {
    _driver = driver;
    _lastKnownBoundingBox = VROBoundingBox(0, 0, 0, 0, 0, 0);

    // Initialize data to something sane
    VROParticlesUBOVertexData vertexData;
    memset(vertexData.particles_transform, 0x0, kMaxParticlesPerUBO * kMaxFloatsPerTransform * sizeof(float));
    
    VROParticlesUBOFragmentData fragmentData;
    memset(fragmentData.frag_particles_color, 0x0, kMaxParticlesPerUBO * kMaxFloatsPerColor * sizeof(float));

    // Set up Vertex UBO
    GL( glGenBuffers(1, &_particleVertexUBO) );
    GL( glBindBuffer(GL_UNIFORM_BUFFER, _particleVertexUBO) );
    GL( glBufferData(GL_UNIFORM_BUFFER, sizeof(VROParticlesUBOVertexData), &vertexData, GL_DYNAMIC_DRAW) );

    // Set up Fragment UBO
    GL( glGenBuffers(1, &_particleFragmentUBO) );
    GL( glBindBuffer(GL_UNIFORM_BUFFER, _particleFragmentUBO) );
    GL( glBufferData(GL_UNIFORM_BUFFER, sizeof(VROParticlesUBOFragmentData), &fragmentData, GL_DYNAMIC_DRAW) );
}

VROParticleUBO::~VROParticleUBO() {
    std::shared_ptr<VRODriverOpenGL> driver = std::dynamic_pointer_cast<VRODriverOpenGL>(_driver.lock());
    if (driver) {
        driver->deleteBuffer(_particleVertexUBO);
        driver->deleteBuffer(_particleFragmentUBO);
    }
}

std::vector<std::shared_ptr<VROShaderModifier>> VROParticleUBO::createInstanceShaderModifier() {
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers;
    std::vector<std::string> vertexModifierCode =  {
            "#include particles_vsh",
            "_transforms.model_matrix = particles_vertex_transform[v_instance_id];",
    };

    // Note: A surface modifier is used here due to a bug in adding shader modifier code:
    // This is because bloom effects are applied before this code is run, resulting in
    // undesired effects. To get around that, we apply a surface modifier instead.
    std::vector<std::string> surfaceModifierCode = {
            "#include particles_fsh",
            "highp vec4 particleColor = particles_fragment_color[v_instance_id];"
            "highp vec4 dest =_surface.diffuse_color.xyzw;",
            "highp vec4 src = particleColor;",

            // Always take 50% of the particle color for now until VIRO-5125 is completed.
            "highp float srcAlpha = 0.5;",
            "if (particleColor.x != -1.0 && _surface.diffuse_color.a != 0.0) {"
            "   highp vec4 final = (src * srcAlpha) + (dest * (1.0 - srcAlpha));",
            "   _surface.diffuse_color.xyz = final.xyz;",
            "}",
            "_surface.alpha = _surface.alpha * particleColor.w;",
    };

    modifiers.push_back(
            std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry, vertexModifierCode));
    modifiers.push_back(
            std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface, surfaceModifierCode));
    return modifiers;
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
    GL( glBindBufferBase(GL_UNIFORM_BUFFER, VROShaderProgram::sParticleVertexUBOBindingPoint, _particleVertexUBO) );
#if VRO_AVOID_BUFFER_SUB_DATA
    GL( glBufferData(GL_UNIFORM_BUFFER, sizeof(VROParticlesUBOVertexData), &vertexData, GL_DYNAMIC_DRAW) );
#else
    GL( glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VROParticlesUBOVertexData), &vertexData) );
#endif
    pglpop();

    pglpush("ParticlesFragment");
    GL( glBindBufferBase(GL_UNIFORM_BUFFER, VROShaderProgram::sParticleFragmentUBOBindingPoint, _particleFragmentUBO) );
#if VRO_AVOID_BUFFER_SUB_DATA
    GL( glBufferData(GL_UNIFORM_BUFFER, sizeof(VROParticlesUBOFragmentData), &fragmentData, GL_DYNAMIC_DRAW) );
#else
    GL( glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VROParticlesUBOFragmentData), &fragmentData) );
#endif
    pglpop();
    return end - start;
}

void VROParticleUBO::update(std::vector<VROParticle> &particles, VROBoundingBox &particleBox) {
    _lastKnownParticles = particles;
    _lastKnownBoundingBox = particleBox;
}

VROBoundingBox VROParticleUBO::getInstancedBoundingBox() {
    return _lastKnownBoundingBox;
}
