//
//  VROWorldUIView.m
//  ViroRenderer
//
//  Created by Raj Advani on 11/2/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#import "VROWorldUIView.h"
#import "VROImageUtil.h"

@interface VROWorldUIView () {
    std::shared_ptr<VROLayer> _layer;
}

@end

@implementation VROWorldUIView

- (instancetype)initWithFrame:(CGRect)frame context:(VRORenderContext *)context {
    self = [super initWithFrame:frame];
    if (self) {
        _layer = std::make_shared<VROLayer>(*context);
        _layer->setFrame(VRORectMake(0, 0, 1.0, 1.0));
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
    _layer->setContents(image);
    
    UIGraphicsEndImageContext();
}

- (std::shared_ptr<VROLayer>) vroLayer {
    return _layer;
}

@end