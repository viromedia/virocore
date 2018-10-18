//
// Created by vik.advani on 10/11/18.
//

#include "VROSceneRenderer.h"
#include "VROProjector.h"

#include <memory>
#include <string>
#include <thread>
#include <vector>

std::vector<VROHitTestResult> VROSceneRenderer::performHitTest(int x, int y, bool boundsOnly) {

    std::vector<VROHitTestResult> results;
    if (_driver != NULL) {
        pinfo("TEST_nativePerformHitTestWithPoint in if(driver) statment of performHitTest");
        if(_driver->getRenderTarget() == NULL) {
            pinfo("TEST_nativePerformHitTestWithPoint  getRenderTarget EQUALS NULL");
        }

        int viewportArr[4] = {0, 0, (int)  getRenderer()->getCamera().getViewport().getWidth(), (int)getRenderer()->getCamera().getViewport().getHeight()};
        VROFieldOfView fov;

        VROMatrix4f projection = getRenderer()->getCamera().getProjection();
        VROMatrix4f mvp = projection.multiply(getRenderer()->getCamera().getLookAtMatrix());

        pinfo("TEST_nativePerformHitTestWithPoint after setting mvsp and projection matrix");

        // unproject the touchPos vector at z = 0 and z = 1
        VROVector3f resultNear;
        VROVector3f resultFar;

        VROProjector::unproject(VROVector3f(x, y, 0), mvp.getArray(), viewportArr, &resultNear);
        VROProjector::unproject(VROVector3f(x, y, 1), mvp.getArray(), viewportArr, &resultFar);
        pinfo("TEST_nativePerformHitTestWithPoint after scalling unproject");
        // minus resultNear from resultFar to get the vector.
        VROVector3f ray = (resultFar - resultNear).normalize();
        std::vector<VROHitTestResult> results = this->performHitTest(getRenderer()->getCamera().getPosition(), ray, boundsOnly);
        return results;
    }

    return results;

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
