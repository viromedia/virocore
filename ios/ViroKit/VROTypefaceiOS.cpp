//
//  VROTypefaceiOS.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROTypefaceiOS.h"
#include "VROLog.h"
#include "VROGlyphOpenGL.h"

#import <UIKit/UIKit.h>

typedef struct FontHeader {
    int32_t fVersion;
    uint16_t fNumTables;
    uint16_t fSearchRange;
    uint16_t fEntrySelector;
    uint16_t fRangeShift;
} FontHeader;

typedef struct TableEntry {
    uint32_t fTag;
    uint32_t fCheckSum;
    uint32_t fOffset;
    uint32_t fLength;
} TableEntry;

VROTypefaceiOS::VROTypefaceiOS(std::string name, int size,
                               std::shared_ptr<VRODriver> driver) :
    VROTypeface(name, size),
    _driver(driver) {
    
    
}

VROTypefaceiOS::~VROTypefaceiOS() {

}

FT_Face VROTypefaceiOS::loadFace(std::string name, int size, FT_Library ft) {
    UIFont *font = [UIFont fontWithName:[NSString stringWithUTF8String:name.c_str()] size:size];
    if (!font) {
        pinfo("Could not find font with name %s, reverting to system font", name.c_str());
        font = [UIFont systemFontOfSize:size];
    }
    
    CFStringRef fontName = (__bridge CFStringRef)[font fontName];
    CGFontRef fontRef = CGFontCreateWithFontName(fontName);
    NSData *fontData = getFontData(fontRef);
    
    FT_Face face;
    if (FT_New_Memory_Face(_ft, (const FT_Byte *)[fontData bytes], [fontData length], 0, &face)) {
        pabort("Failed to load font");
    }
    
    FT_Set_Pixel_Sizes(face, 0, size);
    return face;
}

std::unique_ptr<VROGlyph> VROTypefaceiOS::loadGlyph(FT_ULong charCode, bool forRendering) {
    std::unique_ptr<VROGlyph> glyph = std::unique_ptr<VROGlyph>(new VROGlyphOpenGL());
    glyph->load(_face, charCode, forRendering, _driver.lock());
    
    return glyph;
}

static uint32_t CalcTableCheckSum(const uint32_t *table, uint32_t numberOfBytesInTable) {
    uint32_t sum = 0;
    uint32_t nLongs = (numberOfBytesInTable + 3) / 4;
    while (nLongs-- > 0) {
        sum += CFSwapInt32HostToBig(*table++);
    }
    return sum;
}

NSData *VROTypefaceiOS::getFontData(CGFontRef cgFont) {
    if (!cgFont) {
        return nullptr;
    }
    
    CFRetain(cgFont);
    
    CFArrayRef tags = CGFontCopyTableTags(cgFont);
    CFIndex tableCount = CFArrayGetCount(tags);
    
    size_t *tableSizes = (size_t *) malloc(sizeof(size_t) * tableCount);
    memset(tableSizes, 0, sizeof(size_t) * tableCount);
    
    BOOL containsCFFTable = NO;
    
    size_t totalSize = sizeof(FontHeader) + sizeof(TableEntry) * tableCount;
    
    for (int index = 0; index < tableCount; ++index) {
        
        //get size
        size_t tableSize = 0;
        uintptr_t aTag = (uintptr_t)CFArrayGetValueAtIndex(tags, index);
        
        if (aTag == 'CFF ' && !containsCFFTable) {
            containsCFFTable = YES;
        }
        
        CFDataRef tableDataRef = CGFontCopyTableForTag(cgFont, (uint32_t)aTag);
        if (tableDataRef != NULL) {
            tableSize = CFDataGetLength(tableDataRef);
            CFRelease(tableDataRef);
        }
        totalSize += (tableSize + 3) & ~3;
        
        tableSizes[index] = tableSize;
    }
    
    unsigned char *stream = (unsigned char *) malloc(totalSize);
    
    memset(stream, 0, totalSize);
    char* dataStart = (char*)stream;
    char* dataPtr = dataStart;
    
    // compute font header entries
    uint16_t entrySelector = 0;
    uint16_t searchRange = 1;
    
    while (searchRange < tableCount >> 1) {
        entrySelector++;
        searchRange <<= 1;
    }
    searchRange <<= 4;
    
    uint16_t rangeShift = (tableCount << 4) - searchRange;
    
    // write font header (also called sfnt header, offset subtable)
    FontHeader* offsetTable = (FontHeader*)dataPtr;
    
    //OpenType Font contains CFF Table use 'OTTO' as version, and with .otf extension
    //otherwise 0001 0000
    offsetTable->fVersion = containsCFFTable ? 'OTTO' : CFSwapInt16HostToBig(1);
    offsetTable->fNumTables = CFSwapInt16HostToBig((uint16_t)tableCount);
    offsetTable->fSearchRange = CFSwapInt16HostToBig((uint16_t)searchRange);
    offsetTable->fEntrySelector = CFSwapInt16HostToBig((uint16_t)entrySelector);
    offsetTable->fRangeShift = CFSwapInt16HostToBig((uint16_t)rangeShift);
    
    dataPtr += sizeof(FontHeader);
    
    // write tables
    TableEntry* entry = (TableEntry*)dataPtr;
    dataPtr += sizeof(TableEntry) * tableCount;
    
    for (int index = 0; index < tableCount; ++index) {
        
        uintptr_t aTag = (uintptr_t)CFArrayGetValueAtIndex(tags, index);
        CFDataRef tableDataRef = CGFontCopyTableForTag(cgFont, (uint32_t)aTag);
        size_t tableSize = CFDataGetLength(tableDataRef);
        
        memcpy(dataPtr, CFDataGetBytePtr(tableDataRef), tableSize);
        
        entry->fTag = CFSwapInt32HostToBig((uint32_t)aTag);
        entry->fCheckSum = CFSwapInt32HostToBig(CalcTableCheckSum((uint32_t *)dataPtr, (uint32_t)tableSize));
        
        uint32_t offset = (uint32_t) (dataPtr - dataStart);
        entry->fOffset = CFSwapInt32HostToBig((uint32_t)offset);
        entry->fLength = CFSwapInt32HostToBig((uint32_t)tableSize);
        dataPtr += (tableSize + 3) & ~3;
        ++entry;
        CFRelease(tableDataRef);
    }
    
    CFRelease(cgFont);
    free(tableSizes);
    NSData *fontData = [NSData dataWithBytesNoCopy:stream
                                            length:totalSize
                                      freeWhenDone:YES];
    return fontData;
}
