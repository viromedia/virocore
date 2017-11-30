//
//  VROWeakProxy.m
//  ViroKit
//
//  Created by Andy Chu on 7/18/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import "VROWeakProxy.h"

@implementation VROWeakProxy {
    __weak GLKView *_weakView;
}

+ (VROWeakProxy *)weakProxyForObject:(GLKView *)viewObject {
    return [[VROWeakProxy alloc] initWithView:viewObject];
}

- (instancetype)initWithView:(GLKView *)viewObject {
    self = [super init];
    if (self) {
        _weakView = viewObject;
    }
    return self;
}

- (void)display {
    if (_weakView) {
        [_weakView display];
    }
}

@end
