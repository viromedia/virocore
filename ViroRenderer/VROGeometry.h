//
//  VROGeometry.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROGeometry_h
#define VROGeometry_h

#include <stdio.h>
#include <vector>
#include <string>
#include <memory>
#include "VRORenderContext.h"
#include "VROBoundingBox.h"
#include "VROAllocationTracker.h"

class VROLight;
class VROMaterial;
class VROGeometryElement;
class VROGeometrySource;
class VROGeometrySubstrate;
class VROMatrix4f;
class VRORenderParameters;
enum class VROGeometrySourceSemantic;

/*
 Represents a three-dimensional shape, a collection of vertices, normals and texture coordinates
 that define a surface, also known as a model or mesh. Geometries attached to VRONode objects form
 the visible elements of a scene, and VROMaterial objects attached to a geometry determine its 
 appearance.
 */
class VROGeometry {
    
public:
    
    /*
     Construct a new geometry with the given sources and elements.
     */
    VROGeometry(std::vector<std::shared_ptr<VROGeometrySource>> sources,
                std::vector<std::shared_ptr<VROGeometryElement>> elements) :
        _geometrySources(sources),
        _geometryElements(elements),
        _bounds(nullptr),
        _substrate(nullptr) {
            
         ALLOCATION_TRACKER_ADD(Geometry, 1);
    }
    
    /*
     Copy the given geometry. The materials will *not* be copied, and the
     underlying immutable geometry data will be shared.
     */
    VROGeometry(std::shared_ptr<VROGeometry> geometry) :
        _geometrySources(geometry->_geometrySources),
        _geometryElements(geometry->_geometryElements) {
        
         ALLOCATION_TRACKER_ADD(Geometry, 1);
    }
    
    ~VROGeometry();
    
    void render(const VRORenderContext &context,
                VRORenderParameters &params);
    
    std::vector<std::shared_ptr<VROMaterial>> &getMaterials() {
        return _materials;
    }
    const std::vector<std::shared_ptr<VROMaterial>> &getMaterials_const() const {
        return _materials;
    }
    
    const std::vector<std::shared_ptr<VROGeometrySource>> &getGeometrySources() const {
        return _geometrySources;
    }
    const std::vector<std::shared_ptr<VROGeometryElement>> &getGeometryElements() const {
        return _geometryElements;
    }
    
    const VROBoundingBox &getBoundingBox();
    VROVector3f getCenter();
    
    std::vector<std::shared_ptr<VROGeometrySource>> getGeometrySourcesForSemantic(VROGeometrySourceSemantic semantic) const;
    
private:
    
    /*
     User-assigned name of this geometry.
     */
    std::string _name;
    
    /*
     The materials, which define the surface appearance (color, lighting, texture, and effects)
     of each geometry element.
     
     If a geometry has the same number of materials as it has geometry elements, the material 
     index corresponds to the element index. For geometries with fewer materials than elements,
     the material index for each element is determined by calculating the index of that element
     modulo the number of materials. For example, in a geometry with six elements and three materials, 
     the element at index 5 is rendered using the material at index 5 % 3 = 2.
     */
    std::vector<std::shared_ptr<VROMaterial>> _materials;
    const std::vector<std::shared_ptr<VROGeometrySource>> _geometrySources;
    const std::vector<std::shared_ptr<VROGeometryElement>> _geometryElements;
    
    /*
     The bounding box of this geometry. Created on demand, then cached.
     */
    VROBoundingBox *_bounds;
    
    /*
     Representation of this geometry in the underlying graphics library.
     */
    VROGeometrySubstrate *_substrate;
    
};

#endif /* VROGeometry_h */
