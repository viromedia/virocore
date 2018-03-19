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
#include "VRODriverOpenGLiOS.h"
#import <CoreText/CoreText.h>

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

VROTypefaceiOS::VROTypefaceiOS(std::string name, int size, VROFontStyle style, VROFontWeight weight,
                               std::shared_ptr<VRODriver> driver) :
    VROTypeface(name, size, style, weight),
    _driver(driver),
    _face(nullptr) {
}

VROTypefaceiOS::~VROTypefaceiOS() {
    if (_face != nullptr) {
        FT_Done_Face(_face);
    }
}

void VROTypefaceiOS::loadFace(std::string name, int size) {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (!driver) {
        return;
    }
    
    NSString *familyName = [NSString stringWithUTF8String:name.c_str()];
    
    // Create the symbolic traits bit-mask
    CTFontSymbolicTraits symbolicTraits = 0;
    if (getStyle() == VROFontStyle::Italic) {
        symbolicTraits |= kCTFontTraitItalic;
    }
    
    // CoreText font weight runs from -1.0 to 1.0 (regular is 0.0, which is 400 in our scale)
    CGFloat weight = VROMathInterpolate((float) getWeight(), 100, 900, -1.0, 1.0);
    NSMutableDictionary *attributes = [NSMutableDictionary dictionary];
    [attributes setObject:familyName forKey:(id)kCTFontFamilyNameAttribute];
    
    // The attributes dictionary contains another dictionary, the traits dictionary
    NSMutableDictionary *traits = [NSMutableDictionary dictionary];
    [traits setObject:[NSNumber numberWithUnsignedInt:symbolicTraits] forKey:(id)kCTFontSymbolicTrait];
    [traits setObject:[NSNumber numberWithFloat:weight] forKey:(id)kCTFontWeightTrait];
    [attributes setObject:traits forKey:(id)kCTFontTraitsAttribute];
    [attributes setObject:[NSNumber numberWithFloat:size] forKey:(id)kCTFontSizeAttribute];
    
    // Create the CTFont from the attributes
    CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes((CFDictionaryRef)attributes);
    CTFontRef font = CTFontCreateWithFontDescriptor(descriptor, 0.0, NULL);
    
    // Convert the CTFont to a CGFont and extract the font data
    CGFontRef graphicsFont = CTFontCopyGraphicsFont(font, NULL);
    _fontData = getFontData(graphicsFont);
    
    // Create the FT font from the font data
    FT_Library ft = std::dynamic_pointer_cast<VRODriverOpenGLiOS>(driver)->getFreetype();
    if (FT_New_Memory_Face(ft, (const FT_Byte *)[_fontData bytes], [_fontData length], 0, &_face)) {
        pabort("Failed to load font");
    }
    
    FT_Set_Pixel_Sizes(_face, 0, size);
    
    CFRelease(descriptor);
    CFRelease(font);
    CFRelease(graphicsFont);
}

std::shared_ptr<VROGlyph> VROTypefaceiOS::loadGlyph(FT_ULong charCode, bool forRendering) {
    std::shared_ptr<VROGlyph> glyph = std::make_shared<VROGlyphOpenGL>();
    glyph->load(_face, charCode, forRendering, _driver.lock());
    
    return glyph;
}

float VROTypefaceiOS::getLineHeight() const {
    return _face->size->metrics.height >> 6;
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
    
    free(tableSizes);
    NSData *fontData = [NSData dataWithBytesNoCopy:stream
                                            length:totalSize
                                      freeWhenDone:YES];
    return fontData;
}
