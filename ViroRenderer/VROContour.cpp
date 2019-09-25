//
//  VROContour.cpp
//  ViroKit
//
//  Created by Raj Advani on 7/19/18.
//  Copyright © 2018 Viro Media. All rights reserved.
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
//  Modified from FTGL library
/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 * Copyright (c) 2008 Sam Hocevar <sam@zoy.org>
 * Copyright (c) 2008 Éric Beets <ericbeets@free.fr>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "VROContour.h"
#include <math.h>

void VROContour::addPoint(VROVector3f point) {
    if (pointList.empty() || (point != pointList[pointList.size() - 1]
                              && point != pointList[0])) {
        pointList.push_back(point);
    }

    if (minx > point.x) {
        minx = point.x;
    }
    if (miny > point.y) {
        miny = point.y;
    }
    if (maxx < point.x) {
        maxx = point.x;
    }
    if (maxy < point.y) {
        maxy = point.y;
    }
}


void VROContour::addOutsetPoint(VROVector3f point) {
    outsetPointList.push_back(point);
}

void VROContour::evaluateQuadraticCurve(VROVector3f A, VROVector3f B, VROVector3f C, unsigned short bezierSteps) {
    for(unsigned int i = 1; i < bezierSteps; i++) {
        float t = static_cast<float>(i) / bezierSteps;

        VROVector3f U = (1.0f - t) * A + t * B;
        VROVector3f V = (1.0f - t) * B + t * C;

        addPoint((1.0f - t) * U + t * V);
    }
}

void VROContour::evaluateCubicCurve(VROVector3f A, VROVector3f B, VROVector3f C, VROVector3f D, unsigned short bezierSteps) {
    for(unsigned int i = 0; i < bezierSteps; i++) {
        float t = static_cast<float>(i) / bezierSteps;

        VROVector3f U = (1.0f - t) * A + t * B;
        VROVector3f V = (1.0f - t) * B + t * C;
        VROVector3f W = (1.0f - t) * C + t * D;

        VROVector3f M = (1.0f - t) * U + t * V;
        VROVector3f N = (1.0f - t) * V + t * W;

        addPoint((1.0f - t) * M + t * N);
    }
}

// This function is a bit tricky. Given a path ABC, it returns the
// coordinates of the outset point facing B on the left at a distance
// of 64.0.
//                                         M
//                            - - - - - - X
//                             ^         / '
//                             | 64.0   /   '
//  X---->-----X     ==>    X--v-------X     '
// A          B \          A          B \   .>'
//               \                       \<'  64.0
//                \                       \                  .
//                 \                       \                 .
//                C X                     C X
//
VROVector3f VROContour::computeOutsetPoint(VROVector3f A, VROVector3f B, VROVector3f C) {
    /* Build the rotation matrix from 'ba' vector */
    VROVector3f ba = (A - B).normalize();
    VROVector3f bc = C - B;

    /* Rotate bc to the left */
    VROVector3f tmp(bc.x * -ba.x + bc.y * -ba.y,
                    bc.x * ba.y + bc.y * -ba.x);

    /* Compute the vector bisecting 'abc' */
    double norm = sqrt(tmp.x * tmp.x + tmp.y * tmp.y);
    double dist = 64.0 * sqrt((norm - tmp.x) / (norm + tmp.x));
    tmp.x = (tmp.y < 0.0 ? dist : -dist);
    tmp.y = 64.0;

    /* Rotate the new bc to the right */
    return VROVector3f(tmp.x * -ba.x + tmp.y * ba.y,
                       tmp.x * -ba.y + tmp.y * -ba.x);
}


void VROContour::setParity(int parity) {
    size_t size = getPointCount();
    VROVector3f vOutset;

    if (((parity & 1) && clockwise) || (!(parity & 1) && !clockwise)) {
        // Contour orientation is wrong! We must reverse all points.
        // FIXME: could it be worth writing FTVector::reverse() for this?
        for (size_t i = 0; i < size / 2; i++) {
            VROVector3f tmp = pointList[i];
            pointList[i] = pointList[size - 1 - i];
            pointList[size - 1 -i] = tmp;
        }

        clockwise = !clockwise;
    }

    for(size_t i = 0; i < size; i++) {
        size_t prev, cur, next;

        prev = (i + size - 1) % size;
        cur = i;
        next = (i + size + 1) % size;

        vOutset = computeOutsetPoint(getPoint(prev), getPoint(cur), getPoint(next));
        addOutsetPoint(vOutset);
    }
}

VROVector3f toVec3(FT_Vector &vector) {
    VROVector3f v;
    v.x = vector.x;
    v.y = vector.y;
    v.z = 0;
    return v;
}

VROContour::VROContour(FT_Vector *contour, char* tags, unsigned int n, unsigned short bezierSteps) {
    VROVector3f prev;
    VROVector3f cur = toVec3(contour[(n - 1) % n]);
    VROVector3f next = toVec3(contour[0]);
    VROVector3f a;
    double olddir, dir = atan2((next - cur).y, (next - cur).x);
    double angle = 0.0;

    minx = 65000.0f;
    miny = 65000.0f;
    maxx = -65000.0f;
    maxy = -65000.0f;
    // See http://freetype.sourceforge.net/freetype2/docs/glyphs/glyphs-6.html
    // for a full description of FreeType tags.
    for(unsigned int i = 0; i < n; i++) {
        prev = cur;
        cur = next;
        next = toVec3(contour[(i + 1) % n]);
        olddir = dir;
        dir = atan2((next - cur).y, (next - cur).x);

        // Compute our path's new direction.
        double t = dir - olddir;
        if(t < -M_PI) t += 2 * M_PI;
        if(t > M_PI) t -= 2 * M_PI;
        angle += t;

        // Only process point tags we know.
        if (n < 2 || FT_CURVE_TAG(tags[i]) == FT_Curve_Tag_On) {
            addPoint(cur);
        }
        else if (FT_CURVE_TAG(tags[i]) == FT_Curve_Tag_Conic) {
            VROVector3f prev2 = prev, next2 = next;

            // Previous point is either the real previous point (an "on"
            // point), or the midpoint between the current one and the
            // previous "conic off" point.
            if (FT_CURVE_TAG(tags[(i - 1 + n) % n]) == FT_Curve_Tag_Conic) {
                prev2 = (cur + prev) * 0.5;
                addPoint(prev2);
            }

            // Next point is either the real next point or the midpoint.
            if (FT_CURVE_TAG(tags[(i + 1) % n]) == FT_Curve_Tag_Conic) {
                next2 = (cur + next) * 0.5;
            }

            evaluateQuadraticCurve(prev2, cur, next2, bezierSteps);
        }
        else if (FT_CURVE_TAG(tags[i]) == FT_Curve_Tag_Cubic
                 && FT_CURVE_TAG(tags[(i + 1) % n]) == FT_Curve_Tag_Cubic) {
            evaluateCubicCurve(prev, cur, next, toVec3(contour[(i + 2) % n]), bezierSteps);
        }
    }

    // If final angle is positive (+2PI), it's an anti-clockwise contour,
    // otherwise (-2PI) it's clockwise.
    clockwise = (angle < 0.0);
}
