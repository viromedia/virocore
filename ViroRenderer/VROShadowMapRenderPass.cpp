//
//  VROShadowMapRenderPass.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/15/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROShadowMapRenderPass.h"
#include "VROLog.h"
#include "VROLight.h"
#include "VROOpenGL.h" // For logging pglpush only
#include "VROMaterial.h"
#include "VRORenderTarget.h"
#include "VRODriver.h"
#include "VROPortal.h"
#include "VROScene.h"
#include "VROMath.h"
#include "VRORenderContext.h"
#include "VROPortalFrame.h"
#include "VROShaderModifier.h"

// Shader modifier used for writing to depth buffer
static std::shared_ptr<VROShaderModifier> sShadowDepthWritingModifier;

std::shared_ptr<VROShaderModifier> VROShadowMapRenderPass::getShadowDepthWritingModifier() {
    if (!sShadowDepthWritingModifier) {
        std::vector<std::string> modifierCode =  {
            "_output_color = vec4(0.0, 0.0, _output_color.z, 1.0);",
        };
        sShadowDepthWritingModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Fragment, modifierCode);
    }
    return sShadowDepthWritingModifier;
}

VROShadowMapRenderPass::VROShadowMapRenderPass(const std::shared_ptr<VROLight> light,
                                               std::shared_ptr<VRODriver> driver) :

    _light(light) {
    _silhouetteMaterial = std::make_shared<VROMaterial>();
    _silhouetteMaterial->setWritesToDepthBuffer(true);
    _silhouetteMaterial->setReadsFromDepthBuffer(true);
    _silhouetteMaterial->setCullMode(VROCullMode::None);
    _silhouetteMaterial->addShaderModifier(getShadowDepthWritingModifier());
        
    _shadowTarget = driver->newRenderTarget(VRORenderTargetType::DepthTexture);
    _shadowTarget->setViewport({ 0, 0, _light->getShadowMapSize(), _light->getShadowMapSize() });
}

VROShadowMapRenderPass::~VROShadowMapRenderPass() {
    
}

VRORenderPassInputOutput VROShadowMapRenderPass::render(std::shared_ptr<VROScene> scene, VRORenderPassInputOutput &inputs,
                                                        VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {
    
    // Adjust in case the shadow map size changed
    _shadowTarget->setViewport({ 0, 0, _light->getShadowMapSize(), _light->getShadowMapSize() });

    std::shared_ptr<VRORenderTarget> target = _shadowTarget;
    VROMatrix4f previousProjection = context->getProjectionMatrix();
    VROMatrix4f previousView = context->getViewMatrix();
    
    VROMatrix4f shadowProjection = computeLightProjectionMatrix();
    VROMatrix4f shadowView = computeLightViewMatrix();
    
    context->setProjectionMatrix(shadowProjection);
    context->setViewMatrix(shadowView);
    
    driver->setDepthWritingEnabled(true);
    driver->setColorWritingEnabled(false);
    target->bind();
    target->attachNewTexture();
    target->clearDepth();
    
    // A bit of polygon offset helps combat shadow acne. The first value is slope-dependent, so
    // that we get more polygon offset for pixels that represent a large depth range.
    // In addition to polygon offsetting, we also bias the depth compare in the shaders that
    // read from the map, and use normal/light dot products to special-case surfaces parallel to the light.
    
    // TODO VIRO-1185 control polygon offset through the driver
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 2.0f);
    
    _silhouetteMaterial->bindShader(0, {}, driver);
    _silhouetteMaterial->bindProperties(driver);
    
    std::vector<tree<std::shared_ptr<VROPortal>>> treeNodes;
    treeNodes.push_back(scene->getPortalTree());
    render(treeNodes, target, *context, driver);
    
    // Store generated shadow map properties in the VROLight
    _light->setShadowMap(target->getTexture());
    _light->setShadowViewMatrix(shadowView);
    _light->setShadowProjectionMatrix(shadowProjection);
    
    // Restore state
    glDisable(GL_POLYGON_OFFSET_FILL);
    driver->setColorWritingEnabled(true);
    context->setProjectionMatrix(previousProjection);
    context->setViewMatrix(previousView);
    
    VRORenderPassInputOutput output;
    output[kRenderTargetSingleOutput] = target;
    
    return output;
}

void VROShadowMapRenderPass::render(std::vector<tree<std::shared_ptr<VROPortal>>> &treeNodes,
                                    std::shared_ptr<VRORenderTarget> &target,
                                    const VRORenderContext &context,
                                    std::shared_ptr<VRODriver> &driver) {
    
    int i = 0;
    for (tree<std::shared_ptr<VROPortal>> &treeNode : treeNodes) {
        std::shared_ptr<VROPortal> &portal = treeNode.value;
        pglpush("Shadow Recursion Level %d, Portal %d [%s]", portal->getRecursionLevel(), i, portal->getName().c_str());
        
        portal->renderSilhouettes(_silhouetteMaterial, VROSilhouetteMode::Flat, context, driver);
        render(treeNode.children, target, context, driver);
        
        ++i;
        pglpop();
    }
}

VROMatrix4f VROShadowMapRenderPass::computeLightProjectionMatrix() const {
    float far  = _light->getShadowFarZ();
    float near = _light->getShadowNearZ();
    
    float orthographicScale = _light->getShadowOrthographicScale();
    float left   = -orthographicScale;
    float right  =  orthographicScale;
    float bottom = -orthographicScale;
    float top    =  orthographicScale;
    
    VROMatrix4f lightProjection;
    lightProjection[0]  =  2.0 / (right - left);
    lightProjection[5]  =  2.0 / (top - bottom);
    lightProjection[10] =  2.0 / (far - near);
    lightProjection[12] = -(right + left) / (right - left);
    lightProjection[13] = -(top + bottom) / (top - bottom);
    lightProjection[14] = -(far + near) / (far - near);
    
    return lightProjection;
}

VROMatrix4f VROShadowMapRenderPass::computeLightViewMatrix() const {
    VROVector3f lightForward = _light->getDirection();
    lightForward = lightForward.scale(-1).normalize();
    
    VROMatrix4f rotateX;
    rotateX.rotateX(M_PI_2);
    VROVector3f lightUp = rotateX.multiply(lightForward);
    
    VROVector3f lightEye(0, 0, 0);
    return VROMathComputeLookAtMatrix(lightEye, lightForward, lightUp);
}
