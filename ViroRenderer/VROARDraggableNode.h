//
//  VROARDraggableNode.h
//  ViroKit
//
//  Created by Andy Chu on 8/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARDraggableNode_h
#define VROARDraggableNode_h

#include "VROARNode.h"

/*
 This is a VRONode that can be dragged along real-world surfaces/points. The logic
 for that currently lives within VROInputControllerARiOS.
 */
class VROARDraggableNode : public VRONode {
public:
    VROARDraggableNode();
    virtual ~VROARDraggableNode();
};

#endif /* VROARDraggableNode_h */
