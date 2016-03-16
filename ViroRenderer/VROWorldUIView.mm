//
//  VROWorldUIView.m
//  ViroRenderer
//
//  Created by Raj Advani on 11/2/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#import "VROWorldUIView.h"
#import "VROImageUtil.h"

static const bool kRenderDirectToContext = true;

@interface VROWorldUIView () {
    std::shared_ptr<VROLayer> _layer;
}

@end

@implementation VROWorldUIView

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        _layer = std::make_shared<VROLayer>();
        _layer->setFrame(VRORectMake(0, 0, 1.0, 1.0));
    }
    
    return self;
}

- (void)dealloc {

}

- (void)updateWithContext:(const VRORenderContext *)context {
    if (kRenderDirectToContext) {
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        
        CGFloat scale = [[UIScreen mainScreen] nativeScale];
        int width = self.bounds.size.width * scale;
        int height = self.bounds.size.height * scale;
        
        NSUInteger bytesPerPixel = 4;
        NSUInteger bytesPerRow = bytesPerPixel * width;
        NSUInteger bitsPerComponent = 8;
        
        CGContextRef bitmapContext = CGBitmapContextCreate(nullptr, width, height,
                                                           bitsPerComponent, bytesPerRow, colorSpace,
                                                           kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
        CGColorSpaceRelease(colorSpace);
        
        // Flip since we'll be rendering to a texture
        CGAffineTransform flipVertical = CGAffineTransformMake(1, 0, 0, -1, 0, height);
        CGContextConcatCTM(bitmapContext, flipVertical);
        CGContextScaleCTM(bitmapContext, scale, scale);

        [self.layer renderInContext:bitmapContext];
        _layer->setContents(width, height, bitmapContext, *context);
        
        CGContextRelease(bitmapContext);
    }
    else {
        CGFloat scale = [UIScreen mainScreen].nativeScale;
        UIGraphicsBeginImageContextWithOptions(self.bounds.size, NO, scale);
        
        [self drawViewHierarchyInRect:self.bounds afterScreenUpdates:YES];
        
        UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
        _layer->setContents(image);
        
        UIGraphicsEndImageContext();
    }
}

- (std::shared_ptr<VROLayer>) vroLayer {
    return _layer;
}

@end