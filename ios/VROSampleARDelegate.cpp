//
//  VROSampleARDelegate.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 6/11/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROSampleARDelegate.h"
#include "VROLog.h"

std::shared_ptr<VRONode> VROSampleARDelegate::anchorWasDetected(std::shared_ptr<VROARAnchor> anchor) {
    pinfo("Anchor detected!");
    
    std::shared_ptr<VROARPlaneAnchor> pAnchor = std::dynamic_pointer_cast<VROARPlaneAnchor>(anchor);
    if (pAnchor) {
        pinfo("Plane anchor!");
        pinfo("Center %f, %f, %f", pAnchor->getCenter().x, pAnchor->getCenter().y, pAnchor->getCenter().z);
        pinfo("Extent %f, %f, %f", pAnchor->getExtent().x, pAnchor->getExtent().y, pAnchor->getExtent().z);

        VROMatrix4f transform = anchor->getTransform();
        pinfo("Position %f, %f, %f", transform[12], transform[13], transform[14]);
        
        std::shared_ptr<VRONode> fbxNode = loadFBXNode(pAnchor->getCenter());
        
        /*
         We return a parent anchor node instead of the FBX node directly, because
         the ARSession will set transforms in the parent anchor node, and want to
         maintain our ability to set our own transforms directly in the FBX node.
         */
        std::shared_ptr<VRONode> anchorNode = std::make_shared<VRONode>();
        anchorNode->addChildNode(fbxNode);
        
        return anchorNode;
    }
    else {
        return {};
    }
}

void VROSampleARDelegate::anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor) {
    
}

void VROSampleARDelegate::anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor) {
    
}

void VROSampleARDelegate::anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor) {
    pinfo("Anchor removed");
}

std::shared_ptr<VRONode> VROSampleARDelegate::loadFBXNode(VROVector3f center) {
    NSString *fbxPath = [[NSBundle mainBundle] pathForResource:@"heart" ofType:@"vrx"];
    NSURL *fbxURL = [NSURL fileURLWithPath:fbxPath];
    std::string url = std::string([[fbxURL description] UTF8String]);
    
    NSString *basePath = [fbxPath stringByDeletingLastPathComponent];
    NSURL *baseURL = [NSURL fileURLWithPath:basePath];
    std::string base = std::string([[baseURL description] UTF8String]);
    
    std::shared_ptr<VRONode> fbxNode = VROFBXLoader::loadFBXFromURL(url, base, true,
                                                                    [this, center](std::shared_ptr<VRONode> node, bool success) {
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
