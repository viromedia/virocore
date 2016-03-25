//
//  VROSceneControllerInternal.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/25/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROSceneControllerInternal_h
#define VROSceneControllerInternal_h

#include <memory>

class VROScene;
class VRORenderContext;
class VROHoverDelegate;
class VROHoverController;

class VROSceneControllerInternal {
    
public:
    
    VROSceneControllerInternal();
    virtual ~VROSceneControllerInternal();
    
    std::shared_ptr<VROScene> getScene() const {
        return _scene;
    }
    
    virtual void onSceneWillAppear(VRORenderContext &context);
    virtual void onSceneDidAppear(VRORenderContext &context);
    virtual void onSceneWillDisappear(VRORenderContext &context);
    virtual void onSceneDidDisappear(VRORenderContext &context);
    
    /*
     Set a hover delegate for this scene. Controls the response to hovering
     over objects.
     */
    void setHoverDelegate(std::shared_ptr<VROHoverDelegate> delegate);
    
private:
    
    /*
     The scene managed by this scene controller.
     */
    std::shared_ptr<VROScene> _scene;
    
    /*
     The hover controller used for this scene.
     */
    std::shared_ptr<VROHoverController> _hoverController;
    
};

#endif /* VROSceneControllerInternal_h */
