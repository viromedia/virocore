//
//  VROGeometrySubstrateOpenGL.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROGeometrySubstrateOpenGL_h
#define VROGeometrySubstrateOpenGL_h

#include "VROGeometrySubstrate.h"
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES3/glext.h>

class VRODriverOpenGL;
class VROGeometry;
class VROMaterial;
class VROGeometrySource;
class VROGeometryElement;
class VROMaterialSubstrateOpenGL;
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
};

class VROGeometrySubstrateOpenGL : public VROGeometrySubstrate {
    
public:
    
    VROGeometrySubstrateOpenGL(const VROGeometry &geometry,
                               VRODriverOpenGL &driver);
    virtual ~VROGeometrySubstrateOpenGL();
    
    void render(const VROGeometry &geometry,
                int elementIndex,
                VROMatrix4f transform,
                VROMatrix4f normalMatrix,
                float opacity,
                std::shared_ptr<VROMaterial> &material,
                const VRORenderContext &context,
                VRODriver &driver);
    
private:
    
    std::vector<GLuint> _vaos;
    std::vector<VROGeometryElementOpenGL> _elements;
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
    
    void renderMaterial(VROMaterialSubstrateOpenGL *material,
                        VROGeometryElementOpenGL &element,
                        float opacity,
                        const VRORenderContext &renderContext,
                        VRODriver &driver);
    
};

#endif /* VROGeometrySubstrateOpenGL_h */
