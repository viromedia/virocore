//
//  VROCharmapCoverage.cpp
//  ViroKit
//
//  Created by Raj Advani on 3/21/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

// Modified from CmapCoverage.cpp in Android Minikin project

/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "VROCharmapCoverage.h"
#include <algorithm>
#include <vector>
using std::vector;

#include "VROLog.h"
#include "VROFontUtil.h"
#include "VROSparseBitSet.h"

constexpr uint32_t U32MAX = std::numeric_limits<uint32_t>::max();

// These could perhaps be optimized to use __builtin_bswap16 and friends.
static uint32_t readU16(const uint8_t* data, size_t offset) {
    return ((uint32_t)data[offset]) << 8 | ((uint32_t)data[offset + 1]);
}

static uint32_t readU24(const uint8_t* data, size_t offset) {
    return ((uint32_t)data[offset]) << 16 | ((uint32_t)data[offset + 1]) << 8 |
        ((uint32_t)data[offset + 2]);
}

static uint32_t readU32(const uint8_t* data, size_t offset) {
    return ((uint32_t)data[offset]) << 24 | ((uint32_t)data[offset + 1]) << 16 |
        ((uint32_t)data[offset + 2]) << 8 | ((uint32_t)data[offset + 3]);
}

// The start must be larger than or equal to coverage.back() if coverage is not empty.
// Returns true if the range is appended. Otherwise returns false as an error.
static bool addRange(vector<uint32_t> &coverage, uint32_t start, uint32_t end) {
#ifdef VERBOSE_DEBUG
    pinfo("adding range %d-%d\n", start, end);
#endif
    if (coverage.empty() || coverage.back() < start) {
        coverage.push_back(start);
        coverage.push_back(end);
        return true;
    } else if (coverage.back() == start) {
        coverage.back() = end;
        return true;
    } else {
        // Reject unordered range input since SparseBitSet assumes that the given range vector is
        // sorted. OpenType specification says cmap entries are sorted in order of code point
        // values, thus for OpenType compliant font files, we don't reach here.
        //android_errorWriteLog(0x534e4554, "32178311");
        return false;
    }
}

struct Range {
    uint32_t start;  // inclusive
    uint32_t end;  // exclusive

    static Range InvalidRange() {
        return Range({ U32MAX, U32MAX });
    }

    inline bool isValid() const {
        return start != U32MAX && end != U32MAX;
    }

    // Returns true if left and right intersect.
    inline static bool intersects(const Range& left, const Range& right) {
        return left.isValid() && right.isValid() &&
                left.start < right.end && right.start < left.end;
    }

    // Returns merged range. This method assumes left and right are not invalid ranges and they have
    // an intersection.
    static Range merge(const Range& left, const Range& right) {
        return Range({ std::min(left.start, right.start), std::max(left.end, right.end) });
    }
};

// Returns Range from given ranges vector. Returns InvalidRange if i is out of range.
static inline Range getRange(const std::vector<uint32_t>& r, size_t i) {
    return i + 1 < r.size() ? Range({ r[i], r[i + 1] }) : Range::InvalidRange();
}

// Merge two sorted lists of ranges into one sorted list.
static std::vector<uint32_t> mergeRanges(
        const std::vector<uint32_t>& lRanges, const std::vector<uint32_t>& rRanges) {
    std::vector<uint32_t> out;

    const size_t lsize = lRanges.size();
    const size_t rsize = rRanges.size();
    out.reserve(lsize + rsize);
    size_t ri = 0;
    size_t li = 0;
    while (li < lsize || ri < rsize) {
        Range left = getRange(lRanges, li);
        Range right = getRange(rRanges, ri);

        if (!right.isValid()) {
            // No ranges left in rRanges. Just put all remaining ranges in lRanges.
            do {
                Range r = getRange(lRanges, li);
                addRange(out, r.start, r.end);  // Input is sorted. Never returns false.
                li += 2;
            } while (li < lsize);
            break;
        } else if (!left.isValid()) {
            // No ranges left in lRanges. Just put all remaining ranges in rRanges.
            do {
                Range r = getRange(rRanges, ri);
                addRange(out, r.start, r.end);  // Input is sorted. Never returns false.
                ri += 2;
            } while (ri < rsize);
            break;
        } else if (!Range::intersects(left, right)) {
            // No intersection. Add smaller range.
            if (left.start < right.start) {
                addRange(out, left.start, left.end);  // Input is sorted. Never returns false.
                li += 2;
            } else {
                addRange(out, right.start, right.end);  // Input is sorted. Never returns false.
                ri += 2;
            }
        } else {
            Range merged = Range::merge(left, right);
            li += 2;
            ri += 2;
            left = getRange(lRanges, li);
            right = getRange(rRanges, ri);
            while (Range::intersects(merged, left) || Range::intersects(merged, right)) {
                if (Range::intersects(merged, left)) {
                    merged = Range::merge(merged, left);
                    li += 2;
                    left = getRange(lRanges, li);
                } else {
                    merged = Range::merge(merged, right);
                    ri += 2;
                    right = getRange(rRanges, ri);
                }
            }
            addRange(out, merged.start, merged.end);  // Input is sorted. Never returns false.
        }
    }

    return out;
}

// Get the coverage information out of a Format 4 subtable, storing it in the coverage vector
static bool getCoverageFormat4(vector<uint32_t>& coverage, const uint8_t* data, size_t size) {
    const size_t kSegCountOffset = 6;
    const size_t kEndCountOffset = 14;
    const size_t kHeaderSize = 16;
    const size_t kSegmentSize = 8;  // total size of array elements for one segment
    if (kEndCountOffset > size) {
        return false;
    }
    size_t segCount = readU16(data, kSegCountOffset) >> 1;
    if (kHeaderSize + segCount * kSegmentSize > size) {
        return false;
    }
    for (size_t i = 0; i < segCount; i++) {
        uint32_t end = readU16(data, kEndCountOffset + 2 * i);
        uint32_t start = readU16(data, kHeaderSize + 2 * (segCount + i));
        if (end < start) {
            // invalid segment range: size must be positive
            //android_errorWriteLog(0x534e4554, "26413177");
            return false;
        }
        uint32_t rangeOffset = readU16(data, kHeaderSize + 2 * (3 * segCount + i));
        if (rangeOffset == 0) {
            uint32_t delta = readU16(data, kHeaderSize + 2 * (2 * segCount + i));
            if (((end + delta) & 0xffff) > end - start) {
                if (!addRange(coverage, start, end + 1)) {
                    return false;
                }
            } else {
                for (uint32_t j = start; j < end + 1; j++) {
                    if (((j + delta) & 0xffff) != 0) {
                        if (!addRange(coverage, j, j + 1)) {
                            return false;
                        }
                    }
                }
            }
        } else {
            for (uint32_t j = start; j < end + 1; j++) {
                uint32_t actualRangeOffset = kHeaderSize + 6 * segCount + rangeOffset +
                    (i + j - start) * 2;
                if (actualRangeOffset + 2 > size) {
                    // invalid rangeOffset is considered a "warning" by OpenType Sanitizer
                    continue;
                }
                uint32_t glyphId = readU16(data, actualRangeOffset);
                if (glyphId != 0) {
                    if (!addRange(coverage, j, j + 1)) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

// Get the coverage information out of a Format 12 subtable, storing it in the coverage vector
static bool getCoverageFormat12(vector<uint32_t>& coverage, const uint8_t* data, size_t size) {
    const size_t kNGroupsOffset = 12;
    const size_t kFirstGroupOffset = 16;
    const size_t kGroupSize = 12;
    const size_t kStartCharCodeOffset = 0;
    const size_t kEndCharCodeOffset = 4;
    const size_t kMaxNGroups = 0xfffffff0 / kGroupSize;  // protection against overflow
    // For all values < kMaxNGroups, kFirstGroupOffset + nGroups * kGroupSize fits in 32 bits.
    if (kFirstGroupOffset > size) {
        return false;
    }
    uint32_t nGroups = readU32(data, kNGroupsOffset);
    if (nGroups >= kMaxNGroups || kFirstGroupOffset + nGroups * kGroupSize > size) {
        //android_errorWriteLog(0x534e4554, "25645298");
        return false;
    }
    for (uint32_t i = 0; i < nGroups; i++) {
        uint32_t groupOffset = kFirstGroupOffset + i * kGroupSize;
        uint32_t start = readU32(data, groupOffset + kStartCharCodeOffset);
        uint32_t end = readU32(data, groupOffset + kEndCharCodeOffset);
        if (end < start) {
            // invalid group range: size must be positive
            //android_errorWriteLog(0x534e4554, "26413177");
            return false;
        }

        // No need to read outside of Unicode code point range.
        if (start > MAX_UNICODE_CODE_POINT) {
            return true;
        }
        if (end > MAX_UNICODE_CODE_POINT) {
            // file is inclusive, vector is exclusive
            if (end == 0xFFFFFFFF) {
                //android_errorWriteLog(0x534e4554, "62134807");
            }
            return addRange(coverage, start, MAX_UNICODE_CODE_POINT + 1);
        }
        if (!addRange(coverage, start, end + 1)) {  // file is inclusive, vector is exclusive
            return false;
        }
    }
    return true;
}

// Lower value has higher priority. 0 for the highest priority table.
// kLowestPriority for unsupported tables.
// This order comes from HarfBuzz's hb-ot-font.cc and needs to be kept in sync with it.
constexpr uint8_t kLowestPriority = 255;
uint8_t getTablePriority(uint16_t platformId, uint16_t encodingId) {
    if (platformId == 3 && encodingId == 10) {
        return 0;
    }
    if (platformId == 0 && encodingId == 6) {
        return 1;
    }
    if (platformId == 0 && encodingId == 4) {
        return 2;
    }
    if (platformId == 3 && encodingId == 1) {
        return 3;
    }
    if (platformId == 0 && encodingId == 3) {
        return 4;
    }
    if (platformId == 0 && encodingId == 2) {
        return 5;
    }
    if (platformId == 0 && encodingId == 1) {
        return 6;
    }
    if (platformId == 0 && encodingId == 0) {
        return 7;
    }
    // Tables other than above are not supported.
    return kLowestPriority;
}

// Get merged coverage information from default UVS Table and non-default UVS Table. Note that this
// function assumes code points in both default UVS Table and non-default UVS table are stored in
// ascending order. This is required by the standard.
static bool getVSCoverage(std::vector<uint32_t>* out_ranges, const uint8_t* data, size_t size,
        uint32_t defaultUVSTableOffset, uint32_t nonDefaultUVSTableOffset,
        const VROSparseBitSet& baseCoverage) {
    // Need to merge supported ranges from default UVS Table and non-default UVS Table.
    // First, collect all supported code points from non default UVS table.
    std::vector<uint32_t> rangesFromNonDefaultUVSTable;
    if (nonDefaultUVSTableOffset != 0) {
        constexpr size_t kHeaderSize = 4;
        constexpr size_t kUVSMappingRecordSize = 5;

        const uint8_t* nonDefaultUVSTable = data + nonDefaultUVSTableOffset;
        // This subtraction doesn't underflow since the caller already checked
        // size > nonDefaultUVSTableOffset.
        const size_t nonDefaultUVSTableRemaining = size - nonDefaultUVSTableOffset;
        if (nonDefaultUVSTableRemaining < kHeaderSize) {
            return false;
        }
        const uint32_t numRecords = readU32(nonDefaultUVSTable, 0);
        if (numRecords * kUVSMappingRecordSize + kHeaderSize > nonDefaultUVSTableRemaining) {
            return false;
        }
        for (uint32_t i = 0; i < numRecords; ++i) {
            const size_t recordOffset = kHeaderSize + kUVSMappingRecordSize * i;
            const uint32_t codePoint = readU24(nonDefaultUVSTable, recordOffset);
            if (!addRange(rangesFromNonDefaultUVSTable, codePoint, codePoint + 1)) {
                return false;
            }
        }
    }

    // Then, construct range from default UVS Table with merging code points from non default UVS
    // table.
    std::vector<uint32_t> rangesFromDefaultUVSTable;
    if (defaultUVSTableOffset != 0) {
        constexpr size_t kHeaderSize = 4;
        constexpr size_t kUnicodeRangeRecordSize = 4;

        const uint8_t* defaultUVSTable = data + defaultUVSTableOffset;
        // This subtraction doesn't underflow since the caller already checked
        // size > defaultUVSTableOffset.
        const size_t defaultUVSTableRemaining = size - defaultUVSTableOffset;

        if (defaultUVSTableRemaining < kHeaderSize) {
            return false;
        }
        const uint32_t numRecords = readU32(defaultUVSTable, 0);
        if (numRecords * kUnicodeRangeRecordSize + kHeaderSize > defaultUVSTableRemaining) {
            return false;
        }

        for (uint32_t i = 0; i < numRecords; ++i) {
            const size_t recordOffset = kHeaderSize + kUnicodeRangeRecordSize * i;
            const uint32_t startCp = readU24(defaultUVSTable, recordOffset);
            const uint8_t rangeLength = defaultUVSTable[recordOffset + 3];

            // Then insert range from default UVS Table, but exclude if the base codepoint is not
            // supported.
            for (uint32_t cp = startCp; cp <= startCp + rangeLength; ++cp) {
                // All codepoints in default UVS table should go to the glyphs of the codepoints
                // without variation selectors. We need to check the default glyph availability and
                // exclude the codepoint if it is not supported by defualt cmap table.
                if (baseCoverage.get(cp)) {
                    if (!addRange(rangesFromDefaultUVSTable, cp, cp + 1 /* exclusive */)) {
                        return false;
                    }
                }
            }
        }
    }
    *out_ranges = mergeRanges(rangesFromDefaultUVSTable, rangesFromNonDefaultUVSTable);
    return true;
}

static void getCoverageFormat14(std::vector<std::unique_ptr<VROSparseBitSet>> *out,
        const uint8_t *data, size_t size, const VROSparseBitSet &baseCoverage) {
    constexpr size_t kHeaderSize = 10;
    constexpr size_t kRecordSize = 11;
    constexpr size_t kLengthOffset = 2;
    constexpr size_t kNumRecordOffset = 6;

    out->clear();
    if (size < kHeaderSize) {
        return;
    }

    const uint32_t length = readU32(data, kLengthOffset);
    if (size < length) {
        return;
    }

    uint32_t numRecords = readU32(data, kNumRecordOffset);
    if (numRecords == 0 || kHeaderSize + kRecordSize * numRecords > length) {
        return;
    }

    for (uint32_t i = 0; i < numRecords; ++i) {
        // Insert from the largest code points since it determines the size of the output vector.
        const uint32_t recordHeadOffset = kHeaderSize + kRecordSize * (numRecords - i - 1);
        const uint32_t vsCodePoint = readU24(data, recordHeadOffset);
        const uint32_t defaultUVSOffset = readU32(data, recordHeadOffset + 3);
        const uint32_t nonDefaultUVSOffset = readU32(data, recordHeadOffset + 7);
        if (defaultUVSOffset > length || nonDefaultUVSOffset > length) {
            continue;
        }

        const uint16_t vsIndex = VROFontUtil::getVsIndex(vsCodePoint);
        if (vsIndex == kInvalidVSIndex) {
            continue;
        }
        std::vector<uint32_t> ranges;
        if (!getVSCoverage(&ranges, data, length, defaultUVSOffset, nonDefaultUVSOffset,
                baseCoverage)) {
            continue;
        }
        if (out->size() < vsIndex + 1) {
            out->resize(vsIndex + 1);
        }
        (*out)[vsIndex].reset(new VROSparseBitSet(ranges.data(), ranges.size() >> 1));
    }

    out->shrink_to_fit();
}

VROSparseBitSet VROCharmapCoverage::getCoverage(const uint8_t* cmap_data, size_t cmap_size,
        std::vector<std::unique_ptr<VROSparseBitSet>>* out) {
    constexpr size_t kHeaderSize = 4;
    constexpr size_t kNumTablesOffset = 2;
    constexpr size_t kTableSize = 8;
    constexpr size_t kPlatformIdOffset = 0;
    constexpr size_t kEncodingIdOffset = 2;
    constexpr size_t kOffsetOffset = 4;
    constexpr size_t kFormatOffset = 0;
    constexpr uint32_t kNoTable = UINT32_MAX;

    if (kHeaderSize > cmap_size) {
        return VROSparseBitSet();
    }
    uint32_t numTables = readU16(cmap_data, kNumTablesOffset);
    if (kHeaderSize + numTables * kTableSize > cmap_size) {
        return VROSparseBitSet();
    }

    uint32_t bestTableOffset = kNoTable;
    uint16_t bestTableFormat = 0;
    uint8_t bestTablePriority = kLowestPriority;
    uint32_t vsTableOffset = kNoTable;
    for (uint32_t i = 0; i < numTables; ++i) {
        const uint32_t tableHeadOffset = kHeaderSize + i * kTableSize;
        const uint16_t platformId = readU16(cmap_data, tableHeadOffset + kPlatformIdOffset);
        const uint16_t encodingId = readU16(cmap_data, tableHeadOffset + kEncodingIdOffset);
        const uint32_t offset = readU32(cmap_data, tableHeadOffset + kOffsetOffset);

        if (offset > cmap_size - 2) {
            continue;  // Invalid table: not enough space to read.
        }
        const uint16_t format = readU16(cmap_data, offset + kFormatOffset);

        if (platformId == 0 /* Unicode */ && encodingId == 5 /* Variation Sequences */) {
            if (vsTableOffset == kNoTable && format == 14) {
                vsTableOffset = offset;
            } else {
                // Ignore the (0, 5) table if we have already seen another valid one or it's in a
                // format we don't understand.
            }
        } else {
            uint32_t length;
            uint32_t language;

            if (format == 4) {
                constexpr size_t lengthOffset = 2;
                constexpr size_t languageOffset = 4;
                constexpr size_t minTableSize = languageOffset + 2;
                if (offset > cmap_size - minTableSize) {
                    continue;  // Invalid table: not enough space to read.
                }
                length = readU16(cmap_data, offset + lengthOffset);
                language = readU16(cmap_data, offset + languageOffset);
            } else if (format == 12) {
                constexpr size_t lengthOffset = 4;
                constexpr size_t languageOffset = 8;
                constexpr size_t minTableSize = languageOffset + 4;
                if (offset > cmap_size - minTableSize) {
                    continue;  // Invalid table: not enough space to read.
                }
                length = readU32(cmap_data, offset + lengthOffset);
                language = readU32(cmap_data, offset + languageOffset);
            } else {
                continue;
            }

            if (length > cmap_size - offset) {
                continue;  // Invalid table: table length is larger than whole cmap data size.
            }
            if (language != 0) {
                // Unsupported or invalid table: this is either a subtable for the Macintosh
                // platform (which we don't support), or an invalid subtable since language field
                // should be zero for non-Macintosh subtables.
                continue;
            }
            const uint8_t priority = getTablePriority(platformId, encodingId);
            if (priority < bestTablePriority) {
                bestTableOffset = offset;
                bestTablePriority = priority;
                bestTableFormat = format;
            }
        }
        if (vsTableOffset != kNoTable && bestTablePriority == 0 /* highest priority */) {
            // Already found the highest priority table and variation sequences table. No need to
            // look at remaining tables.
            break;
        }
    }

    // The returned coverage is format 12 if available, otherwise format 4
    // For detail on coverage formats see here:
    // https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6cmap.html
    VROSparseBitSet coverage;

    if (bestTableOffset != kNoTable) {
        const uint8_t* tableData = cmap_data + bestTableOffset;
        const size_t tableSize = cmap_size - bestTableOffset;
        bool success;
        vector<uint32_t> coverageVec;
        if (bestTableFormat == 4) {
            success = getCoverageFormat4(coverageVec, tableData, tableSize);
        } else {
            success = getCoverageFormat12(coverageVec, tableData, tableSize);
        }

        if (success) {
            coverage = VROSparseBitSet(&coverageVec.front(), coverageVec.size() >> 1);
        }
    }

    // If variation coverage is present (format 14) then that is returned in the output
    // vector
    if (vsTableOffset != kNoTable) {
        const uint8_t* tableData = cmap_data + vsTableOffset;
        const size_t tableSize = cmap_size - vsTableOffset;
        getCoverageFormat14(out, tableData, tableSize, coverage);
    }
    return coverage;
}
