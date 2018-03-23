//
//  VROTypefaceCollection.cpp
//  ViroKit
//
//  Created by Raj Advani on 3/21/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROTypefaceCollection.h"
#include "VROTypeface.h"
#include "VROFontUtil.h"

VROTypefaceCollection::VROTypefaceCollection(std::shared_ptr<VROTypeface> typeface) {
    _typefaces = { typeface };
}

VROTypefaceCollection::VROTypefaceCollection(std::vector<std::shared_ptr<VROTypeface>> typefaces) :
    _typefaces(typefaces) {
    
}

VROTypefaceCollection::~VROTypefaceCollection() {
    
}

uint32_t VROTypefaceCollection::computeCoverageScore(std::shared_ptr<VROTypeface> &typeface,
                                                     uint32_t codePoint, uint32_t variationSelector) const {
    // Highest score: typeface has the glyph and variation
    // Medium score: typeface has only base character
    // Zero score: typeface doesn't support the glyph at all
    if (variationSelector != 0) {
        if (typeface->hasCharacter(codePoint, variationSelector)) {
            return 2;
        } else if (typeface->hasCharacter(codePoint, 0)) {
            return 1;
        } else {
            return 0;
        }
    }
    else {
        if (typeface->hasCharacter(codePoint, 0)) {
            return 1;
        } else {
            return 0;
        }
    }
}

std::vector<VROFontRun> VROTypefaceCollection::computeRuns(std::wstring text) {
    std::vector<VROFontRun> runs;
    
    // We only have one typeface in this collection, so don't bother computing runs
    if (_typefaces.size() == 1) {
        runs.push_back({ 0, static_cast<int>(text.size()), _typefaces[0] });
        return runs;
    }
    
    // Iterate through the characters, finding the best typeface for each. Since we're
    // using wide-strings, we expect each character to be a Unicode code point. This is
    // almost always the case, with two complications:
    //
    // 1. Surrogates. Our 2-byte wide characters support all characters in the "Basic
    //    Multilingual Plane" (BMP). There are some characters in Unicode, however, that
    //    require 4 bytes. These are typically mathematical characters, musical symbols,
    //    and rare CJK (Chinese/Japanese/Korean) characters. These are supported in UTF-16
    //    via surrogates. We do not currently support surrogates.
    //
    // 2. Variation sequences. Variation sequences are pairs of code points, a base
    //    character followed by a variation selector. The variation selector is
    //    essentially a 'modifier' that alters the base character. For each code point,
    //    we have to check to see if the _next_ code point is a variation selector. These
    //    are supported by Viro.
    //
    // TODO VIRO-3240 Support surrogate pairs
    std::shared_ptr<VROTypeface> lastTypeface = nullptr;
    int start = 0;
    
    int position = 0;
    for (std::wstring::const_iterator c = text.begin(); c != text.end(); ++c) {
        uint32_t codePoint = *c;
        
        std::wstring::const_iterator n = std::next(c);
        uint32_t nextCodePoint = 0;
        if (n != text.end()) {
            nextCodePoint = *n;
        }
        
        bool shouldContinueRun = false;
        if (VROFontUtil::charDoesNotNeedFontSupport(codePoint)) {
            // Always continue if the code point is a format character not needed to be in the font
            shouldContinueRun = true;
        }
        else if (lastTypeface != nullptr && VROFontUtil::charIsStickyWhitelisted(codePoint)) {
            // Continue using existing font as long as it has coverage and is whitelisted
            shouldContinueRun = lastTypeface->hasCharacter(codePoint, 0);
        }
        
        // If the last typeface does not have the code point (or if the last typeface was null)
        // then find the best typeface
        if (!shouldContinueRun) {
            std::shared_ptr<VROTypeface> bestTypeface;
            int bestTypefaceScore = 0;
            
            for (std::shared_ptr<VROTypeface> &typeface : _typefaces) {
                bool isVariation = VROFontUtil::isVariationSelector(nextCodePoint);
                
                int score = 0;
                if (isVariation) {
                    score = computeCoverageScore(typeface, codePoint, nextCodePoint);
                } else {
                    score = computeCoverageScore(typeface, codePoint, 0);
                }
                
                if (score > bestTypefaceScore) {
                    bestTypeface = typeface;
                    bestTypefaceScore = score;
                }
            }
            
            // The best typeface to use has changed
            if (position == 0 || bestTypeface.get() != lastTypeface.get()) {
                // Close out the last run and start a new range for the new typeface
                if (lastTypeface != nullptr) {
                    runs.push_back({ start, position, lastTypeface });
                    start = position;
                }
                else {
                    // This is the first typeface ever assigned. We are either seeing the very first
                    // character (which means start would already be zero), or we have only seen
                    // characters that don't need any font support (which means we need to adjust
                    // start to be 0 to include those characters)
                    start = 0;
                }
                lastTypeface = bestTypeface;
            }
        }
        ++position;
    }
    
    if (lastTypeface == nullptr) {
        // No character needed any font support, so the font doesn't matter: put it all
        // in one run using the first font
        runs.push_back({ 0, static_cast<int>(text.size()), _typefaces[0] });
    }
    else {
        // Close out the last run
        runs.push_back({ start, static_cast<int>(text.size()), lastTypeface });
    }
    return runs;
}

