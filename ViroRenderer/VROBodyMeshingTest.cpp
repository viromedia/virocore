//
//  VROBodyMeshingTest.cpp
//  ViroSample
//
//  Created by Vik Advani
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROBodyMeshingTest.h"
#include "VROTestUtil.h"
#include "VROModelIOUtil.h"
#include "VROARObjectTargetiOS.h"
#include "VROARDeclarativeObjectNode.h"
#include "VROViewAR.h"
#include "VROSphere.h"


VROBodyMeshingTest::VROBodyMeshingTest() :
VRORendererTest(VRORendererTestType::BodyMeshing) {
    
}

VROBodyMeshingTest::~VROBodyMeshingTest() {
    NSLog(@"Destroying VROBodyMeshingTest");
}

void VROBodyMeshingTest::build(std::shared_ptr<VRORenderer> renderer,
                                    std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                    std::shared_ptr<VRODriver> driver) {
    _is2DHitPoints = true;
    _renderer = renderer;
    std::shared_ptr<VROARSceneController> sceneController = std::make_shared<VROARSceneController>();
    _sceneController = sceneController;
    _sceneController->setDelegate(shared_from_this());
    _arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
    _arScene->initDeclarativeSession();
    
    std::shared_ptr<VRONode> sceneNode = std::make_shared<VRONode>();
    
    _bodyMeshingPoints = std::make_shared<VROARBodyMeshingPointsiOS>();
    _bodyMeshingPoints->initBodyTracking(VROCameraPosition::Back, driver);
    _bodyMeshingPoints->startBodyTracking();
    _bodyMeshingPoints->setDelegate(shared_from_this());
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.6, 0.6, 0.6 });
    _arScene->getRootNode()->addLight(ambient);
 
    _bodyPointsSpheres = std::vector<std::shared_ptr<VRONode>>(20);
    _bodyPointsSpheres.reserve(20);
    int endLoop = static_cast<int>(VROBodyMeshingJoints::kLeftAngle) +1;
    for ( int fooInt = static_cast<int>(VROBodyMeshingJoints::kTop); fooInt !=endLoop; fooInt++ ) {
        std::shared_ptr<VROSphere> sphere = VROSphere::createSphere(0.0002, 20, 20, 20);
        std::shared_ptr<VRONode> sphereNode = std::make_shared<VRONode>();
        sphereNode->setGeometry(sphere);
        sphereNode->setPosition(VROVector3f(0.0f, -2000.0f, -2000.0f));
        _bodyPointsSpheres[fooInt] = sphereNode;
        _arScene->getRootNode()->addChildNode(_bodyPointsSpheres[fooInt]);
    }
}


void VROBodyMeshingTest::onBodyMeshJointsAvail(NSDictionary *joints) {
     CGRect screenBounds = [UIScreen mainScreen].bounds;
      VROViewAR *view = (VROViewAR *)getView();
    
    for(id key in joints) {
        id value = [joints objectForKey:key];
        
        if(value != [NSNull null]) {
            BodyPointImpl *bodyPoint = (BodyPointImpl *)value;
            float contentScaleFactor = view.contentScaleFactor;
            NSLog(@"contentScaleFactor: %f", contentScaleFactor);
            int width =_renderer->getCamera().getViewport().getWidth();
            int height = _renderer->getCamera().getViewport().getHeight();
            
            double x = bodyPoint._point.x * width;
            double y = bodyPoint._point.y * height;

            // take 2d points and extrapolate them by random distance to plot, see if the make a body shape.
            if (_is2DHitPoints) {
                VROVector3f position = _renderer->unprojectPoint(VROVector3f(x,y, 0.001f));
                NSNumber *number = (NSNumber *)key;
                if (bodyPoint._confidence > .4f) {
                    std::shared_ptr<VRONode> vroNode = _bodyPointsSpheres[[number integerValue]];
                    vroNode->setPosition(position);
                }
            } else {
                //fire out 3d hit tests and see if we find 3d body points.
                std::vector<std::shared_ptr<VROARHitTestResult>> results = [view performARHitTestWithPoint:x y:y];
                
                NSMutableArray *returnArray = [[NSMutableArray alloc] initWithCapacity:results.size()];
                for (std::shared_ptr<VROARHitTestResult> &result : results) {
                    VROMatrix4f worldTransform = result->getWorldTransform();
                    VROVector3f translation = worldTransform.extractTranslation();
                    NSNumber *number = (NSNumber *)key;
                    std::shared_ptr<VRONode> vroNode = _bodyPointsSpheres[[number integerValue]];
                    vroNode->setPosition(translation);
                    break;
                }
            }
        }
    }
}


