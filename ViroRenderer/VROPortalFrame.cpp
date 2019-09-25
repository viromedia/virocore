//
//  VROPortalFrame.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/5/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROPortalFrame.h"
#include "VROLineSegment.h"
#include "VROMaterial.h"
#include "VROShaderModifier.h"

// Shader modifier used for alpha discard
static thread_local std::shared_ptr<VROShaderModifier> sAlphaTestModifier;

std::shared_ptr<VROShaderModifier> VROPortalFrame::getAlphaDiscardModifier() {
    if (!sAlphaTestModifier) {
        std::vector<std::string> modifierCode =  {
            "if (_output_color.a > 0.1) discard;",
        };
        sAlphaTestModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Fragment, modifierCode);
        sAlphaTestModifier->setName("alpha-test");
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
    planeNormal = getWorldRotation().multiply(planeNormal);
    
    VROVector3f pointOnPlane = getWorldPosition();
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
