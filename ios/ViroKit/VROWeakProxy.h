//
//  VROWeakProxy.h
//  ViroKit
//
//  Created by Andy Chu on 7/18/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <GLKit/GLKit.h>

@interface VROWeakProxy : NSObject

+ (VROWeakProxy *)weakProxyForObject:(GLKView *)viewObject;
- (instancetype)initWithView:(GLKView *)viewObject;
- (void)display;

@end
