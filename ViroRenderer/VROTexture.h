//
//  VROTexture.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROTexture_h
#define VROTexture_h

#include <UIKit/UIKit.h>
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <memory>
#include <vector>

class VROTextureSubstrate;
class VRODriverContext;
class VROData;

enum class VROTextureType {
    Quad,
    Cube
};

enum class VROTextureFormat {
    ETC2,
    ASTC_4x4_LDR
};

class VROTexture {
    
public:
    
    /*
     Create a new VROTexture with no underlying image data.
     The image data must be injected via setImage*() or setSubstrate().
     */
    VROTexture();
    
    /*
     Create a new VROTexture with the given underlying substrate.
     */
    VROTexture(VROTextureType type, std::unique_ptr<VROTextureSubstrate> substrate);
    
    /*
     Create a new VROTexture from a UIImage. If a render context is supplied, then
     the texture will be prewarmed.
     */
    VROTexture(UIImage *image, const VRODriverContext *context = nullptr);
    VROTexture(std::vector<UIImage *> &images, const VRODriverContext *context = nullptr);
    
    /*
     Create a new VROTexture from the given raw data in the given format.
     */
    VROTexture(VROTextureType type, VROTextureFormat format,
               std::shared_ptr<VROData> data, int width, int height,
               const VRODriverContext *context = nullptr);
    
    virtual ~VROTexture();
    
    void setImage(UIImage *image);
    void setImageCube(UIImage *image);
    void setImageCube(std::vector<UIImage *> &images);
    
    /*
     Get the texture ready for usage now, in advance of when it's visible. If not invoked,
     the texture will be initialized when it is made visible.
     */
    void prewarm(const VRODriverContext &context);
    
    VROTextureSubstrate *const getSubstrate(const VRODriverContext &context);
    void setSubstrate(VROTextureType type, std::unique_ptr<VROTextureSubstrate> substrate);
    
private:
    
    VROTextureType _type;
    
    /*
     The image is retained until the texture is hydrated, after which the
     substrate is populated.
     
     Vector of images is used for cube textures.
     */
    UIImage *_image;
    std::vector<UIImage *> _imagesCube;
    
    /*
     If the underlying texture is compressed, its raw data is retined until the
     substrate is populated.
     */
    std::shared_ptr<VROData> _data;
    VROTextureFormat _format;
    int _width, _height;
    
    /*
     Representation of the texture in the underlying hardware.
     */
    std::unique_ptr<VROTextureSubstrate> _substrate;
    
    /*
     Converts the image(s) into a substrate.
     */
    void hydrate(const VRODriverContext &context);
    
};

#endif /* VROTexture_h */
