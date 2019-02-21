//
//  VROBodyRecognitionTest.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/14/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROBodyRecognitionTest.h"
#include "VROTestUtil.h"
#include "VROSphere.h"

#if VRO_PLATFORM_IOS
#include "VROBodyTrackerYolo.h"
#include "VROBodyTrackeriOS.h"
#include "VRODriverOpenGLiOS.h"

static const int kRecognitionNumColors = 14;
static std::string kPointLabels[14] = {
    "top\t\t\t", //0
    "neck\t\t", //1
    "R shoulder\t", //2
    "R elbow\t\t", //3
    "R wrist\t\t", //4
    "L shoulder\t", //5
    "L elbow\t\t", //6
    "L wrist\t\t", //7
    "R hip\t\t", //8
    "R knee\t\t", //9
    "R ankle\t\t", //10
    "L hip\t\t", //11
    "L knee\t\t", //12
    "L ankle\t\t", //13
};

static UIColor *kColors[14] = {
    [UIColor redColor],
    [UIColor greenColor],
    [UIColor blueColor],
    [UIColor cyanColor],
    [UIColor yellowColor],
    [UIColor magentaColor],
    [UIColor orangeColor],
    [UIColor purpleColor],
    [UIColor brownColor],
    [UIColor blackColor],
    [UIColor darkGrayColor],
    [UIColor lightGrayColor],
    [UIColor whiteColor],
    [UIColor grayColor]
};

#endif

VROBodyRecognitionTest::VROBodyRecognitionTest() :
VRORendererTest(VRORendererTestType::BodyRecognition) {
    
}

VROBodyRecognitionTest::~VROBodyRecognitionTest() {
    
}

void VROBodyRecognitionTest::build(std::shared_ptr<VRORenderer> renderer,
                                   std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                   std::shared_ptr<VRODriver> driver) {
    _renderer = renderer;
    _sceneController = std::make_shared<VROARSceneController>();
    _sceneController->setDelegate(shared_from_this());
    
    _arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
    _arScene->initDeclarativeSession();
    
#if VRO_PLATFORM_IOS
    _view = (VROViewAR *) std::dynamic_pointer_cast<VRODriverOpenGLiOS>(driver)->getView();
    _drawDelegate = [[VROBodyRecognitionDrawDelegate alloc] init];
    [_view setDebugDrawDelegate:_drawDelegate];
    
    std::shared_ptr<VROBodyTracker> tracker = std::make_shared<VROBodyTrackeriOS>();
    tracker->initBodyTracking(VROCameraPosition::Back, driver);
    tracker->startBodyTracking();
    tracker->setDelegate(shared_from_this());
    _bodyTracker = tracker;
#endif
    
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.6, 0.6, 0.6 });
    _arScene->getRootNode()->addLight(ambient);
}

void VROBodyRecognitionTest::onBodyJointsFound(const std::map<VROBodyJointType, std::vector<VROInferredBodyJoint>> &joints) {
#if VRO_PLATFORM_IOS
    int viewWidth  = _view.frame.size.width;
    int viewHeight = _view.frame.size.height;
    
    std::vector<VROVector3f> labelPositions;
    std::vector<NSString *> labels;
    std::vector<VROBoundingBox> boxes;
    std::vector<UIColor *> colors;
    
    for (auto &kv : joints) {
        for (VROInferredBodyJoint joint : kv.second) {
            VROBoundingBox bounds = joint.getBounds();
            
            float x = bounds.getX() * viewWidth;
            float y = bounds.getY() * viewHeight;
            float width = bounds.getSpanX() * viewWidth;
            float height = bounds.getSpanY() * viewHeight;
            
            // Base the color on the X position (keeps color for objects across frames fairly consistent)
            int colorIndex = (int) floor((x / (float) viewWidth) * kRecognitionNumColors);
            colorIndex = MAX(0, MIN(colorIndex, kRecognitionNumColors - 1));
            UIColor *color = kColors[colorIndex];
            
            VROVector3f labelPosition = { (float) (x - width / 2.0), (float) (y + height / 2.0), 0 };
            VROBoundingBox transformedBox(x - width / 2.0, x + width / 2.0, y - height / 2.0, y + height / 2.0, 0, 0);
            
            NSString *className = [NSString stringWithUTF8String:kPointLabels[(int) kv.first].c_str()];
            NSString *classAndConf = [NSString stringWithFormat:@"%@ [%.03f]", className, joint.getConfidence()];
            
            boxes.push_back(transformedBox);
            labels.push_back(classAndConf);
            labelPositions.push_back({ labelPosition.x, labelPosition.y, 0 });
            colors.push_back(color);
        }
    }
    
    [_drawDelegate setBoxes:boxes];
    [_drawDelegate setLabels:labels positions:labelPositions];
    [_drawDelegate setColors:colors];
#endif
}

#if VRO_PLATFORM_IOS
@implementation VROBodyRecognitionDrawDelegate {
    std::vector<VROVector3f> _labelPositions;
    std::vector<NSString *> _labels;
    std::vector<VROBoundingBox> _boxes;
    std::vector<UIColor *> _colors;
}

- (id)init {
    self = [super init];
    if (self) {
        
    }
    return self;
}

- (void)drawRect {
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    UIFont *font = [UIFont boldSystemFontOfSize:16];
    for (int i = 0; i < _labels.size(); i++) {
        VROVector3f point = _labelPositions[i];
        
        CGContextSetRGBFillColor(context, 0, 1, 0, 1);
        CGContextSetRGBStrokeColor(context, 0, 0, 0, 1);
        CGContextSetLineWidth(context, 1);
        
        CGRect rect = CGRectMake(point.x, point.y, 9, 9);
        CGContextAddEllipseInRect(context, rect);
        CGContextDrawPath(context, kCGPathFillStroke);
        
        [_labels[i] drawAtPoint:CGPointMake( point.x, point.y ) withAttributes:@{ NSFontAttributeName:font,
                                                                                  NSForegroundColorAttributeName : _colors[i] } ];
    }
    
    for (int i = 0; i < _boxes.size(); i++) {
        VROBoundingBox box = _boxes[i];
        
        CGRect rect = CGRectMake(box.getX() - box.getSpanX() / 2.0, box.getY() - box.getSpanY() / 2.0, box.getSpanX(), box.getSpanY());
        CGContextAddRect(context, rect);
        CGContextSetStrokeColorWithColor(context, [_colors[i] CGColor]);
        CGContextStrokePath(context);
    }
}

- (void)setLabels:(std::vector<NSString *>)labels positions:(std::vector<VROVector3f>)positions {
    _labels = labels;
    _labelPositions = positions;
}

- (void)setBoxes:(std::vector<VROBoundingBox>)boxes {
    _boxes = boxes;
}

- (void)setColors:(std::vector<UIColor *>)colors {
    _colors = colors;
}

@end
#endif
