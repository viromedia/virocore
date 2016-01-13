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
class VRORenderContext;

enum class VROTextureType {
    Quad,
    Cube
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
     Create a new VROTexture from a UIImage.
     */
    VROTexture(UIImage *image);
    VROTexture(std::vector<UIImage *> &images);
    virtual ~VROTexture();
    
    void setImage(UIImage *image);
    void setImageCube(UIImage *image);
    void setImageCube(std::vector<UIImage *> &images);
    
    VROTextureSubstrate *const getSubstrate(const VRORenderContext &context);
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
     Representation of the texture in the underlying hardware.
     */
    std::unique_ptr<VROTextureSubstrate> _substrate;
    
    /*
     Converts the image(s) into a substrate.
     */
    void hydrate(const VRORenderContext &context);
    
};

#endif /* VROTexture_h */
