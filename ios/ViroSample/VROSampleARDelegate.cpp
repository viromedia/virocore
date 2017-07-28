//
//  VROSampleARDelegate.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 6/11/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROSampleARDelegate.h"
#include "VROLog.h"

void VROSampleARDelegate::anchorWasDetected(std::shared_ptr<VROARAnchor> anchor) {
    pinfo("Anchor detected!");
    
    std::shared_ptr<VROARPlaneAnchor> pAnchor = std::dynamic_pointer_cast<VROARPlaneAnchor>(anchor);
    if (pAnchor) {
        pinfo("Plane anchor!");
        pinfo("Center %f, %f, %f", pAnchor->getCenter().x, pAnchor->getCenter().y, pAnchor->getCenter().z);
        pinfo("Extent %f, %f, %f", pAnchor->getExtent().x, pAnchor->getExtent().y, pAnchor->getExtent().z);

        VROMatrix4f transform = anchor->getTransform();
        pinfo("Position %f, %f, %f", transform[12], transform[13], transform[14]);
        
        std::shared_ptr<VROARNode> anchorNode = std::make_shared<VROARNode>();
        pAnchor->setARNode(anchorNode);
    }
}

void VROSampleARDelegate::onHitResult(VROARHitTestResult result, std::shared_ptr<VROARSession> session, std::shared_ptr<VROScene> scene) {
    if (result.getType() == VROARHitTestResultType::ExistingPlaneUsingExtent ||
        result.getType() == VROARHitTestResultType::ExistingPlane) {
        std::shared_ptr<VRONode> fbxNode = loadCoffeeMug();
        fbxNode->setPosition(result.getLocalTransform().extractTranslation());

        std::shared_ptr<VROARAnchor> anchor = result.getAnchor();
        anchor->getARNode()->addChildNode(fbxNode);
        
        pinfo("Adding FBX to *anchored* plane at local position %f, %f, %f",
              fbxNode->getPosition().x, fbxNode->getPosition().y, fbxNode->getPosition().z);
    }
    else if (false) {
        std::shared_ptr<VRONode> fbxNode = loadCoffeeMug();
        fbxNode->setPosition(result.getWorldTransform().extractTranslation());
        
        scene->getRootNode()->addChildNode(fbxNode);
        pinfo("Adding FBX to unanchored plane at position %f, %f, %f",
              fbxNode->getPosition().x, fbxNode->getPosition().y, fbxNode->getPosition().z);
    }
}

void VROSampleARDelegate::anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor) {
    
}

void VROSampleARDelegate::anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor) {
    
}

void VROSampleARDelegate::anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor) {
    pinfo("Anchor removed");
}

std::shared_ptr<VRONode> VROSampleARDelegate::loadCoffeeMug() {
    NSString *objPath = [[NSBundle mainBundle] pathForResource:@"coffee_mug" ofType:@"obj"];
    NSURL *objURL = [NSURL fileURLWithPath:objPath];
    std::string url = std::string([[objURL description] UTF8String]);
    
    NSString *basePath = [objPath stringByDeletingLastPathComponent];
    NSURL *baseURL = [NSURL fileURLWithPath:basePath];
    std::string base = std::string([[baseURL description] UTF8String]);
    
    
    
    std::shared_ptr<VRONode> objNode = VROOBJLoader::loadOBJFromURL(url, base, true,
                                                                    [this](std::shared_ptr<VRONode> node, bool success) {
                                                                        if (!success) {
                                                                            return;
                                                                        }
                                                                        node->setScale({0.007, 0.007, 0.007});
                                                                        
                                                                        VROTextureInternalFormat format = VROTextureInternalFormat::RGBA8;
                                                                        
                                                                        
                                                                        
                                                                        std::shared_ptr<VROMaterial> material = node->getGeometry()->getMaterials().front();
                                                                        material->getDiffuse().setTexture(std::make_shared<VROTexture>(format, VROMipmapMode::None,
                                                                                                                                       std::make_shared<VROImageiOS>([UIImage imageNamed:@"coffee_mug"], format)));
                                                                        material->getSpecular().setTexture(std::make_shared<VROTexture>(format, VROMipmapMode::None,
                                                                                                                                                                     std::make_shared<VROImageiOS>([UIImage imageNamed:@"coffee_mug_specular"], format)));
                                                                    });
    return objNode;
}

std::shared_ptr<VRONode> VROSampleARDelegate::loadHeart() {
    NSString *fbxPath = [[NSBundle mainBundle] pathForResource:@"heart" ofType:@"vrx"];
    NSURL *fbxURL = [NSURL fileURLWithPath:fbxPath];
    std::string url = std::string([[fbxURL description] UTF8String]);
    
    NSString *basePath = [fbxPath stringByDeletingLastPathComponent];
    NSURL *baseURL = [NSURL fileURLWithPath:basePath];
    std::string base = std::string([[baseURL description] UTF8String]);
    
    std::shared_ptr<VRONode> fbxNode = VROFBXLoader::loadFBXFromURL(url, base, true,
                                                                    [this](std::shared_ptr<VRONode> node, bool success) {
                                                                        if (!success) {
                                                                            return;
                                                                        }
                                                                        node->setScale({0.5, 0.5, 0.5});
                                                                        
                                                                        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                                                                            animateTake(node);
                                                                        });
                                                                    });
    return fbxNode;
}

void VROSampleARDelegate::animateTake(std::shared_ptr<VRONode> node) {
    node->getAnimation("Take 001", true)->execute(node, {});
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        animateTake(node);
    });
}
