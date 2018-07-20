//
//  VROVectorizer.hpp
//  ViroKit
//
//  Created by Raj Advani on 7/19/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//
//  Modified from FTGL library
/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 * Copyright (c) 2008 Éric Beets <ericbeets@free.fr>
 * Copyright (c) 2008 Sam Hocevar <sam@zoy.org>
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

#include "VROVectorizer.h"

VROVectorizer::VROVectorizer(const FT_GlyphSlot glyph, unsigned short bezierSteps)
:   contourList(0),
    ftContourCount(0),
    contourFlag(0) {
    if (glyph) {
        outline = glyph->outline;

        ftContourCount = outline.n_contours;
        contourList = 0;
        contourFlag = outline.flags;

        processContours(bezierSteps);
    }
}

VROVectorizer::~VROVectorizer() {
    for(size_t c = 0; c < getContourCount(); ++c) {
        delete contourList[c];
    }
    delete [] contourList;
}

void VROVectorizer::processContours(unsigned short bezierSteps) {
    short contourLength = 0;
    short startIndex = 0;
    short endIndex = 0;

    contourList = new VROContour*[ftContourCount];

    for(int i = 0; i < ftContourCount; ++i) {
        FT_Vector *pointList = &outline.points[startIndex];
        char *tagList = &outline.tags[startIndex];

        endIndex = outline.contours[i];
        contourLength =  (endIndex - startIndex) + 1;

        VROContour *contour = new VROContour(pointList, tagList, contourLength, bezierSteps);
        contourList[i] = contour;

        startIndex = endIndex + 1;
    }

    // Compute each contour's parity. FIXME: see if FT_Outline_Get_Orientation
    // can do it for us.
    for(int i = 0; i < ftContourCount; i++) {
        VROContour *c1 = contourList[i];

        // 1. Find the leftmost point.
        VROVector3f leftmost(65536.0, 0.0);

        for(size_t n = 0; n < c1->getPointCount(); n++) {
            VROVector3f p = c1->getPoint(n);
            if (p.x < leftmost.x) {
                leftmost = p;
            }
        }

        // 2. Count how many other contours we cross when going further to
        // the left.
        int parity = 0;

        for (int j = 0; j < ftContourCount; j++) {
            if (j == i) {
                continue;
            }

            VROContour *c2 = contourList[j];

            for(size_t n = 0; n < c2->getPointCount(); n++) {
                VROVector3f p1 = c2->getPoint(n);
                VROVector3f p2 = c2->getPoint((n + 1) % c2->getPointCount());

                /* FIXME: combinations of >= > <= and < do not seem stable */
                if ((p1.y < leftmost.y && p2.y < leftmost.y)
                    || (p1.y >= leftmost.y && p2.y >= leftmost.y)
                    || (p1.x > leftmost.x && p2.x > leftmost.x)) {
                    continue;
                }
                else if (p1.x < leftmost.x && p2.x < leftmost.x) {
                    parity++;
                }
                else {
                    VROVector3f a = p1 - leftmost;
                    VROVector3f b = p2 - leftmost;
                    if(b.x * a.y > b.y * a.x) {
                        parity++;
                    }
                }
            }
        }

        // 3. Make sure the glyph has the proper parity.
        c1->setParity(parity);
    }
}

size_t VROVectorizer::getPointCount() {
    size_t s = 0;
    for (size_t c = 0; c < getContourCount(); ++c) {
        s += contourList[c]->getPointCount();
    }
    return s;
}

const VROContour *const VROVectorizer::getContour(size_t index) const {
    return (index < getContourCount()) ? contourList[index] : NULL;
}


