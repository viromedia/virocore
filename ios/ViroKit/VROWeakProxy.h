//
//  VROWeakProxy.h
//  ViroKit
//
//  Created by Andy Chu on 7/18/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import <Foundation/Foundation.h>

@class VROViewAR;

@interface VROWeakProxy : NSObject

+ (VROWeakProxy *)weakProxyForObject:(VROViewAR *)viewObject;

- (instancetype)initWithView:(VROViewAR *)viewObject;

- (void)display;

@end
