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
#include "VRODriverOpenGLMacOS.h"

#include <TargetConditionals.h>
#if TARGET_RT_BIG_ENDIAN
#   define FourCC2Str(fourcc) (const char[]){*((char*)&fourcc), *(((char*)&fourcc)+1), *(((char*)&fourcc)+2), *(((char*)&fourcc)+3),0}
#else
#   define FourCC2Str(fourcc) (const char[]){*(((char*)&fourcc)+3), *(((char*)&fourcc)+2), *(((char*)&fourcc)+1), *(((char*)&fourcc)+0),0}
#endif

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
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (driver && _face != nullptr) {
        // FT crashes if we delete a face after the freetype library has been deleted
        if (std::dynamic_pointer_cast<VRODriverOpenGLMacOS>(driver)->getFreetype() != nullptr) {
            FT_Done_Face(_face);
        }
    }
}

FT_FaceRec_ *VROTypefaceiOS::loadFTFace() {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (!driver) {
        return nullptr;
    }
    
    std::string name = getName();

    NSString *requestedFamily;
    if (!name.empty()) {
        requestedFamily = [NSString stringWithUTF8String:name.c_str()];
    } else {
        // TODO VIRO-3196: The San Francisco font is not rendered with weights correctly
        //                 by Freetype; only its italicized and regular weight versions
        //                 render as expected.
        //                 Additionally, its actual family name ".SF UI Display" is kept
        //                 private and subject to change by Apple. Becuase of these
        //                 complications we use Helvetica Neue as the system font for iOS.
        requestedFamily = @"Helvetica Neue";
    }

    // Try to create the requested font
    CTFontRef font = createFont(requestedFamily, getSize(), getStyle(), getWeight());
    
    NSString *realizedFamily = (__bridge NSString *) CTFontCopyFamilyName(font);
    pinfo("Realized font [name: %@, family: %@] for requested family [%@]",
          CTFontCopyPostScriptName(font),
          realizedFamily, requestedFamily);
    
    // If the font we received differs from the requested one, re-create with the correct
    // weight and style (for some reason if CoreText uses a fallback font it doesn't always
    // transfer the attributes).
    if (![realizedFamily isEqualToString:requestedFamily]) {
        NSLog(@"Realized family [%@] does not match requested family [%@], recreating with requested attributes",
              realizedFamily, requestedFamily);
        
        CTFontRef newFont = createFont(realizedFamily, getSize(), getStyle(), getWeight());
        CFRelease(font);
        font = newFont;
    }
    
    // Get the font table data
    _fontData = getFontData(font);
    
    // Create the FT font from the font data
    FT_Library ft = std::dynamic_pointer_cast<VRODriverOpenGLMacOS>(driver)->getFreetype();
    if (FT_New_Memory_Face(ft, (const FT_Byte *)[_fontData bytes], [_fontData length], 0, &_face)) {
        pabort("Failed to load font");
    }
    
    FT_Set_Pixel_Sizes(_face, 0, getSize());
    CFRelease(font);
    return _face;
}

CTFontRef VROTypefaceiOS::createFont(NSString *family, int size, VROFontStyle style, VROFontWeight weight) {
    // Create the symbolic traits bit-mask
    CTFontSymbolicTraits symbolicTraits = 0;
    if (getStyle() == VROFontStyle::Italic) {
        symbolicTraits |= kCTFontTraitItalic;
    }
    
    // CoreText font weight runs from -1.0 to 1.0 (regular is 0.0, which is 400 in our scale)
    CGFloat w = VROMathInterpolate((float) weight, 100, 900, -1.0, 1.0);
    NSMutableDictionary *attributes = [NSMutableDictionary dictionary];
    [attributes setObject:family forKey:(id)kCTFontFamilyNameAttribute];
    
    // The attributes dictionary contains another dictionary, the traits dictionary
    NSMutableDictionary *traits = [NSMutableDictionary dictionary];
    [traits setObject:[NSNumber numberWithUnsignedInt:symbolicTraits] forKey:(id)kCTFontSymbolicTrait];
    [traits setObject:[NSNumber numberWithFloat:w] forKey:(id)kCTFontWeightTrait];
    [attributes setObject:traits forKey:(id)kCTFontTraitsAttribute];
    [attributes setObject:[NSNumber numberWithInt:size] forKey:(id)kCTFontSizeAttribute];
    
    // Create the CTFont from the attributes
    CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes((CFDictionaryRef)attributes);
    CTFontRef font = CTFontCreateWithFontDescriptor(descriptor, 0.0, NULL);
    CFRelease(descriptor);
    
    return font;
}

std::shared_ptr<VROGlyph> VROTypefaceiOS::loadGlyph(uint32_t charCode, uint32_t variantSelector,
                                                    bool forRendering) {
    std::shared_ptr<VROGlyph> glyph = std::make_shared<VROGlyphOpenGL>();
    glyph->load(_face, charCode, variantSelector, forRendering, _driver.lock());
    
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

NSData *VROTypefaceiOS::getFontData(CTFontRef font) {
    CFArrayRef tags = CTFontCopyAvailableTables(font, kCTFontTableOptionNoOptions);
    CFIndex tableCount = CFArrayGetCount(tags);
    size_t *tableSizes = (size_t *) malloc(sizeof(size_t) * tableCount);
    memset(tableSizes, 0, sizeof(size_t) * tableCount);
    
    BOOL containsCFFTable = NO;
    size_t totalSize = sizeof(FontHeader) + sizeof(TableEntry) * tableCount;
    
    // Every TrueType and OpenType font consists of a header follows by font tables.
    // CoreText exposes these tables so we are able to infer the header and directly
    // copy the tables to create an in-memory byte representation of the font usable
    // by Freetype. The spec for these tables come from both Apple and Microsoft at
    // the following links:
    //
    // https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6head.html
    // https://docs.microsoft.com/en-us/typography/opentype/spec/font-file
    
    for (int index = 0; index < tableCount; ++index) {
        size_t tableSize = 0;
        uintptr_t tag = (uintptr_t)CFArrayGetValueAtIndex(tags, index);
        
        // The 'CFF' table differentiates OpenType from TrueType
        if (tag == 'CFF ' && !containsCFFTable) {
            containsCFFTable = YES;
        }
        
        CFDataRef tableDataRef = CTFontCopyTable(font, (uint32_t)tag, kCTFontTableOptionNoOptions);
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
    
    // Compute font header entries
    uint16_t entrySelector = 0;
    uint16_t searchRange = 1;
    while (searchRange < tableCount >> 1) {
        entrySelector++;
        searchRange <<= 1;
    }
    searchRange <<= 4;
    
    uint16_t rangeShift = (tableCount << 4) - searchRange;
    
    // Write the inferred font header (also called sfnt header or the offset subtable)
    FontHeader *offsetTable = (FontHeader*)dataPtr;
    
    // If we found a CFF table then we have an OpenType font, so use the 'OTTO' version.
    // Otherwise indicate TTF with 0001 0000
    offsetTable->fVersion = containsCFFTable ? 'OTTO' : CFSwapInt16HostToBig(1);
    offsetTable->fNumTables = CFSwapInt16HostToBig((uint16_t)tableCount);
    offsetTable->fSearchRange = CFSwapInt16HostToBig((uint16_t)searchRange);
    offsetTable->fEntrySelector = CFSwapInt16HostToBig((uint16_t)entrySelector);
    offsetTable->fRangeShift = CFSwapInt16HostToBig((uint16_t)rangeShift);
    
    dataPtr += sizeof(FontHeader);
    
    // Write the tables
    TableEntry* entry = (TableEntry*)dataPtr;
    dataPtr += sizeof(TableEntry) * tableCount;
    
    for (int index = 0; index < tableCount; ++index) {
        uintptr_t aTag = (uintptr_t)CFArrayGetValueAtIndex(tags, index);
        CFDataRef tableDataRef = CTFontCopyTable(font, (uint32_t)aTag, kCTFontTableOptionNoOptions);
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
