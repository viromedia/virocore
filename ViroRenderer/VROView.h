//
//  VROView.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/2/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "VRORenderContext.h"
#import "VROLayer.h"
#import <memory>

@interface VROView : UIView

- (instancetype)initWithFrame:(CGRect)frame context:(VRORenderContext *)context;

- (void)update;

@property (readonly, nonatomic) std::shared_ptr<VROLayer> vroLayer;

@end
