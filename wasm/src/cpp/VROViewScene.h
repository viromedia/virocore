//
//  VROViewScene.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/4/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include <memory>

class VRORenderer;
class VROInputControllerWasm;
class VRODriverOpenGLWasm;

class VROViewScene {
public:
    VROViewScene();
    virtual ~VROViewScene();
    
    void drawFrame();
    void update();
    void onResize();
    void onBlur();
    void onFocus();
    
private:
    
    int _frame;
    int _width, _height;
    float _suspendedNotificationTime;
    
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROInputControllerWasm> _inputController;
    std::shared_ptr<VRODriverOpenGLWasm> _driver;
    
};
