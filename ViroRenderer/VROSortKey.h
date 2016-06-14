//
//  VROSortKey.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 6/3/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROSortKey_hpp
#define VROSortKey_hpp

#include <stdio.h>

#define kSortKeySize 40

/*
 Sort keys are used to quickly sort geometry elements into optimal batch rendering order,
 to limit state changes on the GPU. They consist of a large byte[] that can be sorted
 quickly via memcmp.
 */
class VROSortKey {
    
public:
    VROSortKey() {
        memset(key, 0, kSortKeySize);
    }
    
    int compare(const VROSortKey &other) const {
        return memcmp(key, other.key, kSortKeySize);
    }
    
    bool operator==(const VROSortKey& r) const { return compare(r) == 0; }
    bool operator!=(const VROSortKey& r) const { return compare(r) != 0; }
    bool operator< (const VROSortKey& r) const { return compare(r) <  0; }
    bool operator> (const VROSortKey& r) const { return compare(r) >  0; }
    bool operator<=(const VROSortKey& r) const { return compare(r) <= 0; }
    bool operator>=(const VROSortKey& r) const { return compare(r) >= 0; }
    bool operator- (const VROSortKey& r) const { return compare(r); }
    
    union {
        struct {
            
            /*
             Manual rendering order setting is the highest sorting concern.
             */
            uint32_t renderingOrder;
            
            /*
             State-change minimization concerns.
             */
            uint32_t shader;
            uint32_t textures;
            uint32_t lights;
            uint32_t material;
            
            /*
             Tie-breakers, double as pointers to the node and
             index of the geometry element.
             */
            uintptr_t node;
            uint8_t elementIndex;
            
        };
        
        unsigned char key[kSortKeySize];
    };
};

static_assert (kSortKeySize == sizeof(VROSortKey), "Sort key size incorrect");

// Uncomment to see a compiler error indicating the size of each VROSortKey
// template<int s> struct SortKeySize;
// SortKeySize<sizeof(VROSortKey)> sortKeySize;

#endif /* VROSortKey_hpp */
