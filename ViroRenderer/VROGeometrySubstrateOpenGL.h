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
enum class VROGeometryPrimitiveType;


//TODO delete
class VROShaderProgram;

struct VROVertexArrayOpenGL {
    GLuint buffer;
};

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
    int offset;
};

struct VROVertexDescriptorOpenGL {
    GLuint stride;
    VROVertexAttributeOpenGL attributes[10];
};

class VROGeometrySubstrateOpenGL : public VROGeometrySubstrate {
    
public:
    
    VROGeometrySubstrateOpenGL(const VROGeometry &geometry,
                               const VRODriverOpenGL &driver);
    virtual ~VROGeometrySubstrateOpenGL();
    
    void render(const VROGeometry &geometry,
                const std::vector<std::shared_ptr<VROMaterial>> &materials,
                const VRORenderContext &renderContext,
                const VRODriver &driver,
                VRORenderParameters &params);
    
private:
    
    std::vector<VROVertexArrayOpenGL> _vars;
    std::vector<VROGeometryElementOpenGL> _elements;
    
    std::vector<VROVertexDescriptorOpenGL> _vertexDescriptor;
    
    VROShaderProgram *_program;
    
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
     Parse the component type and number of components from the given geometry source.
     */
    std::pair<GLuint, int> parseVertexFormat(std::shared_ptr<VROGeometrySource> &source);

    /*
     Parse a GL primitive type from the given geometry VROGeometryPrimitiveType.
     */
    GLuint parsePrimitiveType(VROGeometryPrimitiveType primitive);
    
};

#endif /* VROGeometrySubstrateOpenGL_h */
