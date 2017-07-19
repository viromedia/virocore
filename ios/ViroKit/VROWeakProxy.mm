//
//  VROWeakProxy.m
//  ViroKit
//
//  Created by Andy Chu on 7/18/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import "VROWeakProxy.h"
#import "VROViewAR.h"

@implementation VROWeakProxy {
    __weak VROViewAR *_weakView;
}

+ (VROWeakProxy *)weakProxyForObject:(VROViewAR *)viewObject {
    return [[VROWeakProxy alloc] initWithView:viewObject];
}

- (instancetype)initWithView:(VROViewAR *)viewObject {
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
