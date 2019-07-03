//
//  VROGeometrySubstrateOpenGL.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROGeometrySubstrateOpenGL_h
#define VROGeometrySubstrateOpenGL_h

#include <map>
#include "VROGeometrySubstrate.h"
#include "VROOpenGL.h"

class VRODriverOpenGL;
class VROGeometry;
class VROMaterial;
class VROGeometrySource;
class VROGeometryElement;
class VROMaterialSubstrateOpenGL;
class VROBoneUBO;
enum class VROGeometryPrimitiveType;

struct VROGeometryElementOpenGL {
    GLuint buffer;
    GLuint primitiveType;
    int indexCount;
    GLuint indexType;
    int indexBufferOffset;
};

struct VROVertexAttributeOpenGL {
    GLuint index;
    GLint size;
    GLenum type;
    uintptr_t offset;
};

struct VROVertexDescriptorOpenGL {
    GLuint buffer;
    GLuint stride;
    int numAttributes;
    VROVertexAttributeOpenGL attributes[10];
    bool ownsBuffer;
};

class VROGeometrySubstrateOpenGL : public VROGeometrySubstrate {
    
public:
    
    VROGeometrySubstrateOpenGL(const VROGeometry &geometry,
                               std::shared_ptr<VRODriverOpenGL> driver);
    virtual ~VROGeometrySubstrateOpenGL();
    
    void update(const VROGeometry &geometry,
                std::shared_ptr<VRODriver> &driver);
    void render(const VROGeometry &geometry,
                int elementIndex,
                VROMatrix4f transform,
                VROMatrix4f normalMatrix,
                float opacity,
                const std::shared_ptr<VROMaterial> &material,
                const VRORenderContext &context,
                std::shared_ptr<VRODriver> &driver);
    
    void renderSilhouette(const VROGeometry &geometry,
                          VROMatrix4f transform,
                          std::shared_ptr<VROMaterial> &material,
                          const VRORenderContext &context,
                          std::shared_ptr<VRODriver> &driver);
    
    void renderSilhouetteTextured(const VROGeometry &geometry,
                                  int element,
                                  VROMatrix4f transform,
                                  std::shared_ptr<VROMaterial> &material,
                                  const VRORenderContext &context,
                                  std::shared_ptr<VRODriver> &driver);
    
private:
    
    std::vector<GLuint> _vaos;
    std::vector<VROGeometryElementOpenGL> _elements;
    std::map<int, std::vector<VROVertexDescriptorOpenGL>> _elementToDescriptorsMap;
    std::vector<VROVertexDescriptorOpenGL> _vertexDescriptors;

    /*
     Parse the given geometry elements and populate the _elements vector with the
     results.
     */
    void readGeometryElements(const std::vector<std::shared_ptr<VROGeometryElement>> &elements);
    
    /*
     Parse the given geometry sources and populate the _vars vector with the
     results.
     */
    void readGeometrySources(const std::vector<std::shared_ptr<VROGeometrySource>> &sources);
    
    /*
     Create a Vertex Array Object for each element.
     */
    void createVAO();

    /*
     Parse the component type and number of components from the given geometry source.
     */
    std::pair<GLuint, int> parseVertexFormat(std::shared_ptr<VROGeometrySource> &source);

    /*
     Parse a GL primitive type from the given geometry VROGeometryPrimitiveType.
     */
    GLuint parsePrimitiveType(VROGeometryPrimitiveType primitive);
    
    /*
     Configure a vertex descriptor using the properties of the given geometry
     sources, each of which corresponds to an attribute.
     */
    VROVertexDescriptorOpenGL configureVertexDescriptor(GLuint buffer, std::vector<std::shared_ptr<VROGeometrySource>> group);

    /*
     Weak reference to the driver that created this substrate. The driver's lifecycle
     is tied to the parent EGL context, so we only delete GL objects if the driver
     is alive, to ensure we're deleting them under the correct context (e.g. to avoid
     accidentally deleting objects in a new context that were created in an older
     one).
     */
    std::weak_ptr<VRODriverOpenGL> _driver;
    
    /*
     The UBO to which we write the latest bone tranforms (if this geometry has a skinner),
     so that they are readable by the GPU.
     */
    std::unique_ptr<VROBoneUBO> _boneUBO;
    
    void renderMaterial(const VROGeometry &geometry,
                        const std::shared_ptr<VROMaterial> &material,
                        VROMaterialSubstrateOpenGL *substrate,
                        VROGeometryElementOpenGL &element,
                        float opacity,
                        const VRORenderContext &renderContext,
                        std::shared_ptr<VRODriver> &driver);
    
};

#endif /* VROGeometrySubstrateOpenGL_h */
