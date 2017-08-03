//
//  VROPortal.h
//  ViroKit
//
//  Created by Raj Advani on 7/31/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROPortal_h
#define VROPortal_h

#include "VRONode.h"
#include "VROTree.h"

/*
 Portals are nodes that partition subgraphs of the overall scene
 graph. They are usedto simulate "teleportation" between two
 different scenes, and can also be used for more efficient visibility
 determination.
 
 Scenes are rendered one portal at a time. That is, we render a portal
 and its subgraph *up to* the next portals we find in the graph, then
 we render those sub-portals and their graphs, and so on.
 */
class VROPortal : public VRONode {
    
public:
    
    VROPortal();
    virtual ~VROPortal();
    
    /*
     Render this portal's background.
     */
    void renderBackground(const VRORenderContext &context, std::shared_ptr<VRODriver> &driver);
    
    /*
     Render this portal's geometry.
     */
    void renderPortal(const VRORenderContext &context, std::shared_ptr<VRODriver> &driver);
    
    /*
     Render this portal's silhouette. The geometry is rendered using the provided material. This
     method is typically used to render into the stencil or depth buffers only.
     */
    void renderPortalSilhouette(std::shared_ptr<VROMaterial> &material,
                                const VRORenderContext &context, std::shared_ptr<VRODriver> &driver);
    
    /*
     Render the visible nodes in this portal's graph, in an order determined by the
     latest computed sort keys.
     */
    void renderContents(const VRORenderContext &context, std::shared_ptr<VRODriver> &driver);
    
    /*
     Iterate up and down the scene graph, starting at the active portal.
     Fill in the provided distance of each portal (defined as steps to
     reach from the active portal), and return all portals reached in the given
     out vector.
     
     This is the recursive helper for VROScene::sortPortalsByDistance().
     
     The frame number if used as a 'visited' flag, as this function searches
     both up and down the graph and is expected to only be invoked once
     per frame as part of the render-cycle.
     */
    void traversePortals(int frame, int recursionLevel, tree<std::shared_ptr<VROPortal>> *outPortals);
    
    /*
     Sort the visible nodes in this portal's sub-graph by their sort-keys, and fill
     the internal _keys vector with the results.
     */
    void sortNodesBySortKeys();
        
    /*
     The portal stencil bits enable the drawing of portals on the screen. This
     field determines:
     
     1. What bit is written to the stencil buffer when rendering this portal's
     direct geometry.
     
     2. What bit this node's *child geometries* must match in the stencil buffer
     in order to pass the stencil test.
     
     In other words, this is the bit that is written to the stencil buffer over
     the area of the portal, and the sub-scene inside the portal will only render
     to the screen where it matches this bit, thereby ensuring we only render the
     subscene over the area of the portal.
     */
    void setStencilBits(int bits) {
        _stencilBits = bits;
    }
    int getStencilBits() const {
        return _stencilBits;
    }
    
    /*
     Represents how how many levels deep this portal is: for example, the active portal
     is 0, a portal within the active portal is 1, a portal within a portal within a portal
     is 2, and so on.
     
     Note this is *always* relative to the active portal.
     */
    int getRecursionLevel() const {
        return _recursionLevel;
    }
    
#pragma mark - Backgrounds
    
    /*
     Note: the scene renders all backgrounds in its tree before rendering
     any content.
     */
    
    /*
     Set the background to a cube-map defined by the given cube texture or
     color.
     */
    void setBackgroundCube(std::shared_ptr<VROTexture> textureCube);
    void setBackgroundCube(VROVector4f color);
    
    /*
     Set the background to a textured sphere.
     */
    void setBackgroundSphere(std::shared_ptr<VROTexture> textureSphere);
    
    /*
     Set the background to an arbitrary geometry. All this guarantees is that
     the given object will be rendered first. No properties will be set on
     this geometry, but typically background geometries are screen-space, and
     do not read or write to the depth buffer.
     */
    void setBackground(std::shared_ptr<VROGeometry> geometry);
    
    /*
     Set an arbitrary transform to apply to the background. The transform
     may also be set as a quaternion (rotation).
     */
    void setBackgroundTransform(VROMatrix4f transform);
    void setBackgroundRotation(VROQuaternion rotation);
    
    std::shared_ptr<VROGeometry> getBackground() const {
        return _background;
    }
    
    void installBackgroundModifier();
    
private:
    
    /*
     See setStencilBits() for description.
     */
    int _stencilBits;
    
    /*
     The number of steps to reach this portal from the active portal.
     */
    int _recursionLevel;
    
    /*
     The nodes in this portal's scene-graph, ordered for rendering by their
     sort keys.
     */
    std::vector<VROSortKey> _keys;
    
    /*
     The background visual to display. All backgrounds in the scene are rendered before
     node content.
     */
    std::shared_ptr<VROGeometry> _background;
    
    /*
     Transform to apply to the background geometry.
     */
    VROMatrix4f _backgroundTransform;
    
};

#endif /* VROPortal_h */
