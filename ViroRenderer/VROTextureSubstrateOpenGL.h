//
//  VROTextureSubstrateOpenGL.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROTextureSubstrateOpenGL_h
#define VROTextureSubstrateOpenGL_h

#import "VROTextureSubstrate.h"
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES3/glext.h>
#import <UIKit/UIKit.h>
#import <memory>
#import <vector>

class VROData;
class VRODriver;
enum class VROTextureType;
enum class VROTextureFormat;

class VROTextureSubstrateOpenGL : public VROTextureSubstrate {
    
public:
    
    /*
     Create a new OpenGL texture out of the contents of the current bitmap
     context.
     */
    VROTextureSubstrateOpenGL(int width, int height, CGContextRef bitmapContext,
                              const VRODriver &driver);
    
    /*
     Create a new Metal texture of the given type from the given images.
     */
    VROTextureSubstrateOpenGL(VROTextureType type, std::vector<UIImage *> &images,
                             const VRODriver &driver);
    
    /*
     Create a new Metal texture out of the given format, with the given width, and height.
     */
    VROTextureSubstrateOpenGL(VROTextureType type, VROTextureFormat format,
                              std::shared_ptr<VROData> data, int width, int height,
                              const VRODriver &driver);
    virtual ~VROTextureSubstrateOpenGL();
    
    GLuint getTexture() const {
        return _texture;
    }
    void setTexture(GLuint texture) {
        _texture = texture;
    }
    
private:
    
    GLuint _texture;
    
};

#endif /* VROTextureSubstrateOpenGL_h */
