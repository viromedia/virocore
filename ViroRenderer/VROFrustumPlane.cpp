//
//  VROFrustumPlane.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
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

#include "VROFrustumPlane.h"

VROFrustumPlane::VROFrustumPlane() {

}

VROFrustumPlane::~VROFrustumPlane() {

}

void VROFrustumPlane::refreshFarPoints() {
    /*
     Compute the AABB far points for this plane. The far points are a function of this plane's normal. The positive
     far point is the point furthest from the plane in the direction of the normal; and the negative far point is the
     point furthest from the plane in the opposite direction of the normal.
     
     These points are used to accelerate frustum/AABB intersection tests.
     
     Specifically, if the positive far point is in the negative half-space of the plane, then we know the entire AABB
     is in the negative half-space of the plane. Since the frustum planes point inward, the negative half-space of each
     plane corresponds to being *outside* the frustum. We therefore know that if the positive far point is in the negative
     space of the frustum plane, then the entire bounding box is outside the frustum.
     
     The following is a diagram showing how this works, where ABCD is a bounding box and P1/P2/P3/P4 is a frustum:
     
             P3
         /-------/
        /    A--/----B
    P4 /     | / P2  |
      /      |/      |
     /-------/       |
        P1   |       |
             C-------D
     
     In the above diagram, look at P4. We see its normal points down and to the right. We can also see that if D is in
     the *negative* half space of P4 (that is, in the direction outside of the frustum from P4), then we know for sure
     that the *entire box* is outside P4 -- that's because D is the point of any AABB that's furthest in the interior of
     P4. We can use that knowledge to dramatically reduce the number of plane/point checks required to identify a 
     frustum/AABB intersection. The same goes for P2 and A, P3 and C, and P1 and B. The negative far points can also be
     used, to quickly distinguish between intersection and total containment.
     
     The following example shows how we can quickly eliminate a frustum/bounding-box intersection by noting that the positive
     far point of P2, which is A, is in the negative half-space of P2, thereby implying the entire box must be outside the
     frustum:

          P3
        /-------/
       /       /
  P4  /       /    A------B
     /       / P2  |      |
    /       /      |      |
   /-------/       |      |
      P1           C------D


     Note that this far point optimization breaks down if we have a gigantic bounding box off in the distance that does
     not contain the frustum. For example:
     
     A||B
      ||         P3
      ||      /----/
      ||  P4 /    /  P2
      ||    /----/
      ||      P1
      ||
      ||
      ||
      ||
     C  D
     
     In the above diagram, the positive far points are:
     
     P1: B
     P2: A
     P3: C
     P4: D
     
     The far point algorithm implies that if any positive far point is on the *negative* (outer, or out of the frustum) side of
     the plane, then the box as a whole is outside the frustum. Otherwise, the box intersects or is contained by the plane.
     
     Checking with the above diagram:
     
     B is on the positive side of P1
     A is on the positive side of P2
     C is on the positive side of P3
     D is on the positive side of P4
     
     So clearly, here, the far points optimization returns the *wrong* result. This seems at odds with the pseudocode in this
     paper: http://jesper.kalliope.org/blog/library/vfcullbox.pdf. Figure that out, somebody?
     */
    VROVector3f &N = normal;
    if (N.x > 0) {
        farPoints[VROFarPointPosX] = VROBoxPlaneMaxX;
        farPoints[VROFarPointNegX] = VROBoxPlaneMinX;
    }
    else {
        farPoints[VROFarPointPosX] = VROBoxPlaneMinX;
        farPoints[VROFarPointNegX] = VROBoxPlaneMaxX;
    }
    if (N.y > 0) {
        farPoints[VROFarPointPosY] = VROBoxPlaneMaxY;
        farPoints[VROFarPointNegY] = VROBoxPlaneMinY;
    }
    else {
        farPoints[VROFarPointPosY] = VROBoxPlaneMinY;
        farPoints[VROFarPointNegY] = VROBoxPlaneMaxY;
    }
    if (N.z > 0) {
        farPoints[VROFarPointPosZ] = VROBoxPlaneMaxZ;
        farPoints[VROFarPointNegZ] = VROBoxPlaneMinZ;
    } else {
        farPoints[VROFarPointPosZ] = VROBoxPlaneMinZ;
        farPoints[VROFarPointNegZ] = VROBoxPlaneMaxZ;
    }
}
