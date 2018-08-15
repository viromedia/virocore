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
#include "VROMaterial.h"
#include "VRORenderTarget.h"
#include "VRODriver.h"
#include "VROPortal.h"
#include "VROScene.h"
#include "VROMath.h"
#include "VRORenderContext.h"
#include "VROPortalFrame.h"
#include "VROShaderModifier.h"
#include "VROPencil.h"
#include "VROBoneUBO.h"
#include "VROFieldOfView.h"

// Shader modifier used for writing to depth buffer
static thread_local std::shared_ptr<VROShaderModifier> sShadowDepthWritingModifier;

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
    _silhouetteStaticMaterial = std::make_shared<VROMaterial>();
    _silhouetteStaticMaterial->setWritesToDepthBuffer(true);
    _silhouetteStaticMaterial->setReadsFromDepthBuffer(true);
    _silhouetteStaticMaterial->setCullMode(VROCullMode::None);
    _silhouetteStaticMaterial->addShaderModifier(getShadowDepthWritingModifier());
        
    _silhouetteSkeletalMaterial = std::make_shared<VROMaterial>();
    _silhouetteSkeletalMaterial->setWritesToDepthBuffer(true);
    _silhouetteSkeletalMaterial->setReadsFromDepthBuffer(true);
    _silhouetteSkeletalMaterial->setCullMode(VROCullMode::None);
    _silhouetteSkeletalMaterial->addShaderModifier(getShadowDepthWritingModifier());
    _silhouetteSkeletalMaterial->addShaderModifier(VROBoneUBO::createSkinningShaderModifier(true));
}

VROShadowMapRenderPass::~VROShadowMapRenderPass() {
    
}

void VROShadowMapRenderPass::render(std::shared_ptr<VROScene> scene,
                                    std::shared_ptr<VROScene> outgoingScene,
                                    VRORenderPassInputOutput &inputs,
                                    VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {
    std::shared_ptr<VRORenderTarget> target = inputs.outputTarget;
    VROMatrix4f previousProjection = context->getProjectionMatrix();
    VROMatrix4f previousView = context->getViewMatrix();
    
    VROMatrix4f shadowProjection = computeLightProjectionMatrix();
    VROMatrix4f shadowView = computeLightViewMatrix();
    
    context->setProjectionMatrix(shadowProjection);
    context->setViewMatrix(shadowView);
    
    driver->setDepthWritingEnabled(true);
    driver->setRenderTargetColorWritingMask(VROColorMaskNone);
    driver->bindRenderTarget(target, VRORenderTargetUnbindOp::Invalidate);
    target->clearDepth();
    
    std::vector<tree<std::shared_ptr<VROPortal>>> treeNodes;
    treeNodes.push_back(scene->getPortalTree());
    
    // Render static objects
    _silhouetteStaticMaterial->bindShader(0, {}, *context, driver);
    _silhouetteStaticMaterial->bindProperties(driver);
    render(treeNodes, target, _silhouetteStaticMaterial, [this](const VRONode &node)->bool {
        if ((_light->getInfluenceBitMask() & node.getShadowCastingBitMask()) == 0) {
            return false;
        }
        // Don't use this material for skeletal animation models; use the next one
        if (node.getGeometry() == nullptr || node.getGeometry()->getSkinner().get() != nullptr) {
            return false;
        }
        // If any material doesn't cast a shadow, don't cast for the whole node (technical limitation)
        for (const std::shared_ptr<VROMaterial> &material : node.getGeometry()->getMaterials()) {
            if (!material->getCastsShadows()) {
                return false;
            }
        }
        return true;
    }, *context, driver);
    
    // Render skeletal animation objects
    _silhouetteSkeletalMaterial->bindShader(0, {}, *context, driver);
    _silhouetteSkeletalMaterial->bindProperties(driver);
    render(treeNodes, target, _silhouetteSkeletalMaterial, [this](const VRONode &node)->bool {
        if ((_light->getInfluenceBitMask() & node.getShadowCastingBitMask()) == 0) {
            return false;
        }
        // We should be using the previous material for non-skeletal models
        if (node.getGeometry() == nullptr || node.getGeometry()->getSkinner().get() == nullptr) {
            return false;
        }
        // If any material doesn't cast a shadow, don't cast for the whole node (technical limitation)
        for (const std::shared_ptr<VROMaterial> &material : node.getGeometry()->getMaterials()) {
            if (!material->getCastsShadows()) {
                return false;
            }
        }
        return true;
    }, *context, driver);
    
    // Store generated shadow map properties in the VROLight
    _light->setShadowViewMatrix(shadowView);
    _light->setShadowProjectionMatrix(shadowProjection);

    if (kDrawShadowFrusta) {
        drawShadowFrusta(scene, context, driver);
    }
    
    // Restore state
    driver->setRenderTargetColorWritingMask(VROColorMaskAll);
    context->setProjectionMatrix(previousProjection);
    context->setViewMatrix(previousView);
}

void VROShadowMapRenderPass::render(std::vector<tree<std::shared_ptr<VROPortal>>> &treeNodes,
                                    std::shared_ptr<VRORenderTarget> &target,
                                    std::shared_ptr<VROMaterial> material,
                                    std::function<bool(const VRONode&)> filter,
                                    const VRORenderContext &context,
                                    std::shared_ptr<VRODriver> &driver) {
    
    int i = 0;
    for (tree<std::shared_ptr<VROPortal>> &treeNode : treeNodes) {
        std::shared_ptr<VROPortal> &portal = treeNode.value;
        pglpush("Shadow Recursion Level %d, Portal %d [%s]", portal->getRecursionLevel(), i, portal->getName().c_str());
        
        portal->renderSilhouettes(material, VROSilhouetteMode::Flat, filter, context, driver);
        render(treeNode.children, target, material, filter, context, driver);
        
        ++i;
        pglpop();
    }
}

VROMatrix4f VROShadowMapRenderPass::computeLightProjectionMatrix() const {
    float near = _light->getShadowNearZ();
    float far  = _light->getShadowFarZ();
    
    if (_light->getType() == VROLightType::Directional) {
        float orthographicHalfSize = _light->getShadowOrthographicSize() / 2.0;
        float left   = -orthographicHalfSize;
        float right  =  orthographicHalfSize;
        float bottom = -orthographicHalfSize;
        float top    =  orthographicHalfSize;
        
        return VROMathComputeOrthographicProjection(left, right, bottom, top, near, far);
    }
    else if (_light->getType() == VROLightType::Spot) {
        return VROMathComputePerspectiveProjection(_light->getSpotOuterAngle(), 1, near, far);
    }
    else {
        pabort("Light of type %d may not cast shadows", (int)_light->getType());
    }
}

VROMatrix4f VROShadowMapRenderPass::computeLightViewMatrix() const {
    VROVector3f lightForward = _light->getTransformedDirection().normalize();
    
    VROMatrix4f rotateX;
    rotateX.rotateX(M_PI_2);
    VROVector3f lightUp = rotateX.multiply(lightForward);
    
    return VROMathComputeLookAtMatrix(_light->getTransformedPosition(), lightForward, lightUp);
}

void VROShadowMapRenderPass::drawShadowFrusta(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                              std::shared_ptr<VRODriver> &driver) {
    const std::vector<std::shared_ptr<VROLight>> &lights = scene->getLights();
    for (const std::shared_ptr<VROLight> &light : lights) {
        if (!light->getCastsShadow()) {
            continue;
        }
        light->drawLightFrustum(context->getPencil());
    }
}
