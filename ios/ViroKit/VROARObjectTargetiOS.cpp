//
//  VROARObjectTargetiOS.cpp
//  ViroKit
//
//  Created by Andy Chu on 8/8/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROARObjectTargetiOS.h"
#include <ARKit/ARKit.h>

VROARObjectTargetiOS::VROARObjectTargetiOS(NSURL *localFileUrl) :
    _localFileUrl(localFileUrl) {
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 120000
    if (@available(iOS 12.0, *)) {
        NSError *error = nil;
        _referenceObject = [[ARReferenceObject alloc] initWithArchiveURL:_localFileUrl error:&error];
        if (error) {
            NSLog(@"[Viro] Error creating object target: [%@]", [error localizedDescription]);
        }
    }
#endif
}

VROARObjectTargetiOS::~VROARObjectTargetiOS() {
    
}

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 120000
ARReferenceObject *VROARObjectTargetiOS::getARReferenceObject() {
    return _referenceObject;
}

#endif
