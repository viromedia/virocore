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
#include "VROLineSegment.h"

/*
 Portals are nodes that partition subgraphs of the overall scene
 graph. They are used to simulate "teleportation" between two
 different scenes, and can also be used for more efficient visibility
 determination.
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
     Fill in the recursion level of each portal (defined as steps to
     reach from the active portal), and return a graph of all portals reached.
     
     This is the recursive helper for VROScene::sortPortalsByDistance().
     
     The frame number if used as a 'visited' flag, as this function searches
     both up and down the graph and is expected to only be invoked once
     per frame as part of the render-cycle.
     
     The portalWindowToRender flag is used to determine what portal geometry a portal
     should render. See the discussion under _portalWindowToRender for more detail.
     */
    void traversePortals(int frame, int recursionLevel,
                         std::shared_ptr<VROPortal> portalWindowToRender,
                         tree<std::shared_ptr<VROPortal>> *outPortals);
    
    /*
     Sort the visible nodes in this portal's sub-graph by their sort-keys, and fill
     the internal _keys vector with the results.
     */
    void sortNodesBySortKeys();
    
    /*
     Represents how how many levels deep this portal is: for example, the active portal
     is 0, a portal within the active portal is 1, a portal within a portal within a portal
     is 2, and so on.
     
     Note this is *always* relative to the active portal.
     */
    int getRecursionLevel() const {
        return _recursionLevel;
    }
    
    /*
     If using a two-sided portal (see _twoSided below) this returns the face that we
     should currently be rendering to the stencil buffer. The other face (the inactive)
     face, will not render to the stencil buffer. This achieves the desired effect of
     creating a two-sided portal (the active face will display one world, the inactive
     face the other).
     */
    VROFace getActiveFace() {
        if (isWindowEgress()) {
            if (_portalWindowToRender->isTwoSided()) {
                return VROFace::Back;
            }
            else {
                return VROFace::FrontAndBack;
            }
        }
        else {
            if (_twoSided) {
                return VROFace::Front;
            }
            else {
                return VROFace::FrontAndBack;
            }
        }
    }
    VROFace getInactiveFace() {
        VROFace active = getActiveFace();
        switch (active) {
            case VROFace::Front:
                return VROFace::Back;
            case VROFace::Back:
                return VROFace::Front;
            default:
                return VROFace::FrontAndBack;
        }
    }
    
    /*
     Return true if the portal window rendered by this portal is an exit into
     a parent portal, as opposed to this portal's own 'entrance' portal.
     */
    bool isWindowEgress() const {
        return _portalWindowToRender != nullptr && _portalWindowToRender.get() != this;
    }
    
    /*
     Return true if this portal is two-sided. See _twoSided below for discussion.
     */
    bool isTwoSided() const {
        return _twoSided;
    }
    void setTwoSided(bool twoSided) {
        _twoSided = twoSided;
    }
    
    /*
     If a portal is passable then it reprsents an entry-point into another navigable world,
     meaning it can be set as the active portal. Non-passable portals can be AR elements overlaid
     on the real world like picture frames, that are meant to be viewed from outside only.
     */
    void setPassable(bool passable) {
        _passable = passable;
    }
    bool isPassable() const {
        return _passable;
    }
    
    /*
     Return true if the given line segment intersects the portal geometry.
     */
    bool intersectsLineSegment(VROLineSegment segment) const;
    
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
    
private:
    
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
     True if this portal can be entered; e.g, if it can be made into an
     active portal.
     */
    bool _passable;
    
    /*
     The VROPortal that contains the actual *geometry* we should render for
     this portal's window. This is typically this portal's own geometry,
     except when rendering a portal that leads to a destination 'up' the tree.
     In those cases, we want the parent portal (the destination) to use the
     child portal's geometry for the window.
     
     That is, a portal's own geometry is only ever used as an entrance to
     that portal's content. However, when we step through a portal (when we've
     'entered' the portal), we need to render an exit back out to the parent.
     The parent renders the exit as the child portal's geometry.
     */
    std::shared_ptr<VROPortal> _portalWindowToRender;
    
    /*
     Normally, both sides of a portal are windows into the same scene. So if
     you are in an AR world and you're looking through a portal into a VR world,
     you can walk around the portal and see the VR world from both sides of the
     portal. However, it's also useful to sometimes have 2-sided portals: that is,
     a portal which on one side shows the VR world and the other side the AR world.
     This is particularly useful when we're about to pass through a portal. In such
     cases, we want the portal to show different worlds on each side in order to make
     the transition not flicker.
     
     Two-sided portals are enabled under the hood by using the front/back stencil
     buffer operations.
     */
    bool _twoSided;
    
    /*
     The background visual to display. All backgrounds in the scene are rendered before
     node content.
     */
    std::shared_ptr<VROGeometry> _background;
    
    /*
     Transform to apply to the background geometry.
     */
    VROMatrix4f _backgroundTransform;
    
    /*
     Installs required shader modifiers on the background.
     */
    void installBackgroundModifier();
    
};

#endif /* VROPortal_h */
