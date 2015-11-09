//
//  VROView.m
//  ViroRenderer
//
//  Created by Raj Advani on 11/2/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#import "VROView.h"
#import "VROImageUtil.h"

@interface VROView () {
    std::shared_ptr<VROLayer> _layer;
}

@end


@implementation VROView

- (instancetype)initWithFrame:(CGRect)frame context:(VRORenderContext *)context {
    self = [super initWithFrame:frame];
    if (self) {
        _layer = std::make_shared<VROLayer>(*context);
        _layer->setFrame(VRORectMake(0, 0, 1.0, 1.0));
        _layer->setBackgroundColor({ 1.0, 1.0, 1.0, 1.0 });
    }
    
    return self;
}

- (void)dealloc {

}

- (void)update {
    CGFloat scale = [UIScreen mainScreen].nativeScale;
    UIGraphicsBeginImageContextWithOptions(self.bounds.size, NO, scale);
    
    [self drawViewHierarchyInRect:self.bounds afterScreenUpdates:YES];
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    
    size_t dataLength;
    void *data = VROExtractRGBA8888FromImage(image, &dataLength);
    _layer->setContents(data, dataLength,
                        image.size.width * scale,
                        image.size.height * scale);
    
    UIGraphicsEndImageContext();
}

- (std::shared_ptr<VROLayer>) vroLayer {
    return _layer;
}

@end