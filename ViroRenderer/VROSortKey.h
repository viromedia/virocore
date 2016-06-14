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
    
    /*
     Returns true if this sort key is in the same 'rendering batch' as another
     sort-key; this is the case if all the render-order and state-change components
     of the sort-keys are equal, without inspecting the tiebreaker component.
     
     If the material ID of this sort-key is zero, then we always return false,
     because direct materials are never batched. Note we don't have to check the
     other's material ID because the memcmp ensures that this.material=other.material.
     
     Note that the material buffer mask we use is assuming little-endian order, which is
     why it appears reversed (i.e. in big-endian it would be 0xFFFFFF00).
     */
    bool isSameBatch(const VROSortKey& other) const {
#ifdef UPN_ARCH_64_BIT
        // There are 4 bytes of padding in 64-bit: 12 body + 4 padding + 8 tie-breaker = 24
        return memcmp(key, other.key, sizeof(VROSortKey) - sizeof(size_t) - 4) == 0 && material != 0;
#else
        return memcmp(key, other.key, sizeof(VROSortKey) - sizeof(size_t)) == 0 && material != 0;
#endif
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
             Tie-breakers, double as pointer to the node and
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
