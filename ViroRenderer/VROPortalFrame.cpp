//
//  VROPortalFrame.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/5/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPortalFrame.h"
#include "VROLineSegment.h"
#include "VRORenderTarget.h"
#include "VROShaderModifier.h"

// Shader modifier used for alpha discard
static std::shared_ptr<VROShaderModifier> sAlphaTestModifier;

std::shared_ptr<VROShaderModifier> VROPortalFrame::getAlphaDiscardModifier() {
    if (!sAlphaTestModifier) {
        std::vector<std::string> modifierCode =  {
            "if (_output_color.a > 0.1) discard;",
        };
        sAlphaTestModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Fragment, modifierCode);
    }
    return sAlphaTestModifier;
}

VROPortalFrame::VROPortalFrame() :
    _twoSided(false) {
    _type = VRONodeType::PortalFrame;
    
}

VROPortalFrame::~VROPortalFrame() {
    
}

bool VROPortalFrame::intersectsLineSegment(VROLineSegment segment) const {
    /*
     Perform a line-segment intersection with the plane.
     */
    VROVector3f planeNormal(0, 0, 1);
    planeNormal = getComputedRotation().multiply(planeNormal);
    
    VROVector3f pointOnPlane = getComputedPosition();
    VROVector3f intersectionPt;
    bool intersection = segment.intersectsPlane(pointOnPlane, planeNormal, &intersectionPt);
    
    /*
     Check if our portal contains the intersection point.
     */
    if (intersection) {
        return getUmbrellaBoundingBox().containsPoint(intersectionPt);
    }
    else {
        return false;
    }
}

VROFace VROPortalFrame::getActiveFace(bool isExit) const {
    if (_twoSided) {
        if (isExit) {
            return VROFace::Back;
        }
        else {
            return VROFace::Front;
        }
    }
    else {
        return VROFace::FrontAndBack;
    }
}

VROFace VROPortalFrame::getInactiveFace(bool isExit) const {
    VROFace active = getActiveFace(isExit);
    switch (active) {
        case VROFace::Front:
            return VROFace::Back;
        case VROFace::Back:
            return VROFace::Front;
        default:
            return VROFace::FrontAndBack;
    }
}
