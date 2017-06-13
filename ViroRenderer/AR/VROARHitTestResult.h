//
//  VROARHitTestResult.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 6/12/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARHitTestResult_hpp
#define VROARHitTestResult_hpp

#include <stdio.h>

/*
 Types of hit test results:
 
 ExistingPlaneUsingExtent: Hit test found a plane for which we have an anchor,
                           and the hit location was within the plane's estimated
                           extent.
 ExistingPlane: Hit test found a plane for which we have an anchor, but our hit
                test did not take into account the estimated extent. The hit point
                may be outside the actual extent of the surface.
 EstimatedHorizontalPlane: Hit test found a plane, but one for which we have no 
                           anchor.
 Feature Pont: Hit test found a point that the AR session believes is part of a 
               continuous surface. This surface may not be horizontal.
 */
enum class VROARHitTestResultType {
    ExistingPlaneUsingExtent,
    ExistingPlane,
    EstimatedHorizontalPlane,
    FeaturePoint,
};

/*
 Return value of AR hit tests. AR hit tests determine anchors or 
 less defined features the user hits in the camera view.
 */
class VROARHitTestResult {
public:
    
    VROARHitTestResult(VROARHitTestResultType type, std::shared_ptr<VROARAnchor> anchor, float distance, VROMatrix4f transform) :
        _type(type), _anchor(anchor), _distance(distance), _transform(transform) {}
    
    /*
     Get the type of hit test result.
     */
    VROARHitTestResultType getType() const { return _type; }
    
    /*
     Return the anchor associated with the hit test, if any.
     */
    const std::shared_ptr<VROARAnchor> getAnchor() const { return _anchor; }
    
    /*
     Get the distance from the camera to the hit test result.
     */
    float getDistance() const { return _distance; }
    
    /*
     Get the position and orientation of the hit test result surface, in matrix form.
     */
    VROMatrix4f getTransform() const { return _transform; }
    
private:
    
    VROARHitTestResultType _type;
    std::shared_ptr<VROARAnchor> _anchor;
    float _distance;
    VROMatrix4f _transform;
    
};

#endif /* VROARHitTestResult_hpp */
