//
//  Created by vik.advani on 10/11/18.
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

#include "VROSceneRenderer.h"
#include "VROProjector.h"

#include <memory>
#include <string>
#include <thread>
#include <vector>

std::vector<VROHitTestResult> VROSceneRenderer::performHitTest(int x, int y, bool boundsOnly) {
    std::vector<VROHitTestResult> results;
    if (!_driver) {
        return results;
    }

    int viewportArr[4] = {0, 0, (int) getRenderer()->getCamera().getViewport().getWidth(),
                                (int) getRenderer()->getCamera().getViewport().getHeight()};
    VROMatrix4f projection = getRenderer()->getCamera().getProjection();
    VROMatrix4f mvp = projection.multiply(getRenderer()->getCamera().getLookAtMatrix());

    // Unproject the touch vector at z = 0 and z = 1
    VROVector3f resultNear;
    VROVector3f resultFar;

    VROProjector::unproject(VROVector3f(x, y, 0), mvp.getArray(), viewportArr, &resultNear);
    VROProjector::unproject(VROVector3f(x, y, 1), mvp.getArray(), viewportArr, &resultFar);

    // Subtract resultNear from resultFar to get the vector
    VROVector3f ray = (resultFar - resultNear).normalize();
    return this->performHitTest(getRenderer()->getCamera().getPosition(), ray, boundsOnly);
}

std::vector<VROHitTestResult> VROSceneRenderer::performHitTest(VROVector3f origin, VROVector3f ray, bool boundsOnly) {
    std::vector<VROHitTestResult> results;
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    std::shared_ptr<VROPortal> sceneRootNode = scene->getRootNode();
    // Grab all the nodes that were hit
    std::vector<VROHitTestResult> nodeResults = sceneRootNode->hitTest(_renderer->getCamera(), origin, ray, boundsOnly);
    results.insert(results.end(), nodeResults.begin(), nodeResults.end());

    // Sort and get the closest node
    std::sort(results.begin(), results.end(), [](VROHitTestResult a, VROHitTestResult b) {
        return a.getDistance() < b.getDistance();
    });

    return results;
}
