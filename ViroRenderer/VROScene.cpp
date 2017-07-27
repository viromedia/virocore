//
//  VROScene.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/19/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//
#include <algorithm>
#include "VROScene.h"
#include "VRORenderContext.h"
#include "VRONode.h"
#include "VROGeometry.h"
#include "VROInputControllerBase.h"
#include "VROLight.h"
#include "VROHitTestResult.h"
#include "VROMaterial.h"
#include "VROLog.h"
#include "VROAudioPlayer.h"
#include "VROSurface.h"
#include <stack>
#include <algorithm>

VROScene::VROScene() : VROThreadRestricted(VROThreadName::Renderer) {
    ALLOCATION_TRACKER_ADD(Scenes, 1);
}

VROScene::~VROScene() {
    ALLOCATION_TRACKER_SUB(Scenes, 1);
}

void VROScene::renderBackground(const VRORenderContext &renderContext,
                                std::shared_ptr<VRODriver> &driver) {
    passert_thread();
    for (std::shared_ptr<VRONode> &node : _nodes) {
        node->renderBackground(renderContext, driver);
    }
}

void VROScene::render(const VRORenderContext &context,
                      std::shared_ptr<VRODriver> &driver) {
    passert_thread();
    
    uint32_t boundShaderId = UINT32_MAX;
    std::vector<std::shared_ptr<VROLight>> boundLights;
    
    if (kDebugSortOrder) {
        pinfo("Rendering");
    }
    
    for (VROSortKey &key : _keys) {
        VRONode *node = (VRONode *)key.node;
        int elementIndex = key.elementIndex;
        
        const std::shared_ptr<VROGeometry> &geometry = node->getGeometry();
        if (geometry) {
            std::shared_ptr<VROMaterial> material = geometry->getMaterialForElement(elementIndex);
            if (!key.incoming) {
                material = material->getOutgoing();
            }
            
            if (key.shader != boundShaderId) {
                material->bindShader(driver);
                boundShaderId = key.shader;
                
                // If the shader changes, we have to rebind the lights so they attach
                // to the new shader
                material->bindLights(key.lights, node->getComputedLights(), context, driver);
                boundLights = node->getComputedLights();
            }
            else {
                // Otherwise we only rebind lights if the lights themselves have changed
                if (boundLights != node->getComputedLights()) {
                    material->bindLights(key.lights, node->getComputedLights(), context, driver);
                    boundLights = node->getComputedLights();
                }
            }
            
            // Only render the material if there are lights, or if the material uses
            // constant lighting. Non-constant materials do not render unless we have
            // at least one light.
            if (!boundLights.empty() || material->getLightingModel() == VROLightingModel::Constant) {
                if (kDebugSortOrder) {
                    if (node->getGeometry() && elementIndex == 0) {
                        pinfo("   Rendering node [%s], element %d", node->getGeometry()->getName().c_str(), elementIndex);
                    }
                }
                node->render(elementIndex, material, context, driver);
            }
        }
    }
}

void VROScene::computeTransforms(const VRORenderContext &context) {
    for (std::shared_ptr<VRONode> &node : _nodes) {
        node->computeTransforms({}, {});
    }
}

void VROScene::updateVisibility(const VRORenderContext &context) {
    for (std::shared_ptr<VRONode> &node : _nodes) {
        node->updateVisibility(context);
    }
}

void VROScene::applyConstraints(const VRORenderContext &context) {
    for (std::shared_ptr<VRONode> &node : _nodes) {
        node->applyConstraints(context, {}, false);
    }
}

void VROScene::updateSortKeys(const VRORenderContext &context, std::shared_ptr<VRODriver> &driver) {
    passert_thread();
    
    if (kDebugSortOrder) {
        pinfo("Updating sort keys");
        VRONode::resetDebugSortIndex();
    }

    VRORenderParameters renderParams;
    for (std::shared_ptr<VRONode> &node : _nodes) {
        node->collectLights(&renderParams.lights);
    }
    for (std::shared_ptr<VRONode> &node : _nodes) {
        node->updateSortKeys(0, renderParams, context, driver);
    }
    
    _keys.clear();
    for (std::shared_ptr<VRONode> &node : _nodes) {
        node->getSortKeysForVisibleNodes(&_keys);
    }
    
    std::sort(_keys.begin(), _keys.end());
    _distanceOfFurthestObjectFromCamera = renderParams.furthestDistanceFromCamera;
}

void VROScene::addNode(std::shared_ptr<VRONode> node) {
    passert_thread();
    _nodes.push_back(node);
}

void VROScene::detachInputController(std::shared_ptr<VROInputControllerBase> controller){
    passert_thread();
    if (!_controllerPresenter){
        return;
    }

    std::shared_ptr<VRONode> node = _controllerPresenter->getRootNode();
    auto it = std::find(_nodes.begin(), _nodes.end(), node);
    if (it != _nodes.end()){
        _nodes.erase(it);
    }

    controller->detachScene();
    _controllerPresenter = nullptr;
}

void VROScene::attachInputController(std::shared_ptr<VROInputControllerBase> controller) {
    passert_thread();

    std::shared_ptr<VROInputPresenter> presenter = controller->getPresenter();
    if (_controllerPresenter == presenter){
        return;
    }

    std::shared_ptr<VRONode> node = presenter->getRootNode();
    _nodes.push_back(node);
    _controllerPresenter = presenter;

    controller->attachScene(shared_from_this());
}

std::shared_ptr<VROInputPresenter> VROScene::getControllerPresenter(){
    return _controllerPresenter;
}

std::vector<std::shared_ptr<VROGeometry>> VROScene::getBackgrounds() {
    std::vector<std::shared_ptr<VROGeometry>> backgrounds;
    for (std::shared_ptr<VRONode> &node : _nodes) {
        getBackgrounds(node, backgrounds);
    }
    return backgrounds;
}

void VROScene::getBackgrounds(std::shared_ptr<VRONode> node, std::vector<std::shared_ptr<VROGeometry>> &backgrounds) {
    if (node->getBackground() != nullptr) {
        backgrounds.push_back(node->getBackground());
    }
    
    for (std::shared_ptr<VRONode> &child : node->getSubnodes()) {
        getBackgrounds(child, backgrounds);
    }
}

