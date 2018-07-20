//
//  VROTextFormatter.hpp
//  ViroKit
//
//  Created by Raj Advani on 7/19/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROTextFormatter_h
#define VROTextFormatter_h

#include <vector>
#include <map>
#include <string>
#include <memory>

class VROGlyph;
enum class VROTextClipMode;
enum class VROTextHorizontalAlignment;
enum class VROTextVerticalAlignment;
enum class VROLineBreakMode;

class VROTextLine {
public:
    std::wstring line;
    float spacingRatio;
    
    VROTextLine(std::wstring &line) : line(line), spacingRatio(1) {}
    VROTextLine(std::wstring &line, float spacingRatio) : line(line), spacingRatio(spacingRatio) {}
    virtual ~VROTextLine() {}
};

/*
 Methods for processing word-wrapping and paragraph processing. These are used by
 both VROText and VROText3D.
 */
class VROTextFormatter {
public:
    
    /*
     Format the text with the given parameters, and invoke the buildFunction on each glyph
     as it is laid out, providing its computed X and Y position. The buildFunction can be used
     to create the geometry for the completed text object. When this function returns, all
     glyphs will have had the buildFunction invoked for them, and outRealizedWidth and
     outRealizedHeight will be populated with the final width and height of the text.
     */
    static void formatAndBuild(std::wstring &text, float maxWidth, float maxHeight, int maxLines, float lineHeight,
                               VROTextHorizontalAlignment horizontalAlignment,
                               VROTextVerticalAlignment verticalAlignment,
                               VROLineBreakMode lineBreakMode, VROTextClipMode clipMode,
                               std::map<uint32_t, std::shared_ptr<VROGlyph>> &glyphMap,
                               float *outRealizedWidth, float *outRealizedHeight,
                               std::function<void(std::shared_ptr<VROGlyph> &glyph, float x, float y)> buildFunction);
    
    /*
     Simple methods for processing the line-break mode. All of the methods below use a
     'greedy' algorithm, filling as much space in the current line as possible then moving
     to the next line. These methods also introduce a newline on hard breaks (i.e. whenever
     the '\n' character is encountered). In particular, the wrapByNewlines function *only*
     processes hard breaks; the rest process both hard and soft.
     
     These functions also handle clipping. When char/word wrapping is on, we only have to
     clip text vertically (horizontal edges are implicitly taken care of by the wrapping
     function). When char/word wrapping is off, we also have to clip text horizontally.
     */
    static std::vector<VROTextLine> wrapByWords(std::wstring &text,
                                                float maxWidth, float maxHeight, int maxLines, float lineHeight,
                                                VROTextClipMode clipMode,
                                                std::map<uint32_t, std::shared_ptr<VROGlyph>> &glyphMap);
    static std::vector<VROTextLine> wrapByChars(std::wstring &text,
                                                float maxWidth, float maxHeight, int maxLines, float lineHeight,
                                                VROTextClipMode clipMode,
                                                std::map<uint32_t, std::shared_ptr<VROGlyph>> &glyphMap);
    static std::vector<VROTextLine> wrapByNewlines(std::wstring &text,
                                                   float maxWidth, float maxHeight, int maxLines, float lineHeight,
                                                   VROTextClipMode clipMode,
                                                   std::map<uint32_t, std::shared_ptr<VROGlyph>> &glyphMap);
    
    /*
     Justification routine. Considerably more complex than the greedy algorithms above. Note that
     justification is a word-wrapping technique that reduces the 'raggedness' of the text edges;
     it can be used with left, right, and centered horizontal alignment. To achieve traditional
     justified text as seen in newspapers, use it with VROTextHorizontalAlignment::Left.
     */
    static std::vector<VROTextLine> justify(std::wstring &text,
                                            float maxWidth, float maxHeight, int maxLines, float lineHeight,
                                            VROTextClipMode clipMode,
                                            std::map<uint32_t, std::shared_ptr<VROGlyph>> &glyphMap);
    
    /*
     Helpers for wrapping/clipping.
     */
    static std::vector<std::wstring> divideIntoParagraphs(std::wstring &text);
    static float getLengthOfWord(const std::wstring &word, std::map<uint32_t, std::shared_ptr<VROGlyph>> &glyphMap);
    
    static bool isAnotherLineAvailable(size_t numLinesNow, float maxHeight, int maxLines,
                                       float lineHeight, VROTextClipMode clipMode);

};

#endif /* VROTextFormatter_h */
