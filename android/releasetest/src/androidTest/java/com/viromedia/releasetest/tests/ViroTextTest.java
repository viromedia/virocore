//
//  Copyright (c) 2017-present, ViroMedia, Inc.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

package com.viromedia.releasetest.tests;

import android.graphics.Color;
import android.support.test.espresso.core.deps.guava.collect.Iterables;

import com.viro.core.AmbientLight;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Text;
import com.viro.core.Vector;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

/**
 * Created by manish on 11/7/17.
 */

public class ViroTextTest extends ViroBaseTest {

    private static final String TEST_STRING = "The quick brown fox jumps over the lazy dog";
    private Node mTextNode;
    private Text mText;

    @Override
    void configureTestScene() {
        final AmbientLight ambientLight = new AmbientLight(Color.WHITE, 1000);
        mScene.getRootNode().addLight(ambientLight);

        mText = Text.builder().viroContext(mViroView.getViroContext()).textString(TEST_STRING).
                fontFamilies("Roboto").fontSize(25).color(Color.WHITE).width(2.5f).height(1f).
                horizontalAlignment(Text.HorizontalAlignment.LEFT).verticalAlignment(Text.VerticalAlignment.TOP).
                clipMode(Text.ClipMode.NONE).lineBreakMode(Text.LineBreakMode.WORD_WRAP).build();
        mTextNode = new Node();
        mTextNode.setPosition(new Vector(0f, 0f, -3.3f));
        mTextNode.setGeometry(mText);
        mScene.getRootNode().addChildNode(mTextNode);
    }

    @Test
    public void testText() {
        runUITest(() -> testSetText());
        runUITest(() -> testSetFontFamily());
        runUITest(() -> testSetFontSize());
        runUITest(() -> testSetFontStyle());
        runUITest(() -> testSetFontWeight());
        runUITest(() -> testInternationalization());
        runUITest(() -> testSetColor());
        runUITest(() -> testSetWidth());
        runUITest(() -> testSetHeight());
        runUITest(() -> testSetHorizontalAlignment());
        runUITest(() -> testSetVerticalAlignment());
        runUITest(() -> testSetLineBreakMode());
        runUITest(() -> testSetClipMode());
        runUITest(() -> testSetMaxLines());
        runUITest(() -> testExtrusion());
        runUITest(() -> testExtrusionColors());
        runUITest(() -> testExtrusionAltering());
        runUITest(() -> testOutline());
        runUITest(() -> testDropShadow());
        runUITest(() -> testOutlineColorChange());
        runUITest(() -> testThickOutline());
        runUITest(() -> testAlternatingOuterStroke());
    }

    private void testSetLineBreakMode() {
        mText.setText(TEST_STRING);

        final Iterator<Text.LineBreakMode> itr = Iterables.cycle(Text.LineBreakMode.values()).iterator();
        mMutableTestMethod = () -> {
            mText.setLineBreakMode(itr.next());
        };
        assertPass("Cycling through all LineBreakMode values", () -> {
            mText.setLineBreakMode(Text.LineBreakMode.WORD_WRAP);
        });
    }

    private void testSetClipMode() {
        final Iterator<Text.ClipMode> itr = Iterables.cycle(Text.ClipMode.values()).iterator();
        mMutableTestMethod = () -> {
            mText.setClipMode(itr.next());
        };
        assertPass("Cycling through all ClipMode values", () -> {
            mText.setClipMode(Text.ClipMode.NONE);
        });
    }

    private void testSetMaxLines() {
        final List<Integer> maxLines = Arrays.asList(0, 1, 2, 3, 4, 5);
        final Iterator<Integer> itr = Iterables.cycle(maxLines).iterator();

        mMutableTestMethod = () -> {
            mText.setMaxLines(itr.next());
        };
        assertPass("Cycling through increasing maxLines", () -> {
            mText.setMaxLines(0);
        });
    }

    private void testSetVerticalAlignment() {
        final Iterator<Text.VerticalAlignment> itr = Iterables.cycle(Text.VerticalAlignment.values()).iterator();
        mMutableTestMethod = () -> {
            mText.setVerticalAlignment(itr.next());
        };
        assertPass("Cycling through all VerticleAlignment values", () -> {
            mText.setVerticalAlignment(Text.VerticalAlignment.TOP);
        });
    }

    private void testSetHorizontalAlignment() {
        final Iterator<Text.HorizontalAlignment> itr = Iterables.cycle(Text.HorizontalAlignment.values()).iterator();
        mMutableTestMethod = () -> {
            mText.setHorizontalAlignment(itr.next());
        };
        assertPass("Cycling through all HorizontalAlignment values", () -> {
            mText.setHorizontalAlignment(Text.HorizontalAlignment.LEFT);
        });
    }

    private void testSetHeight() {
        final List<Integer> heights = Arrays.asList(1, 2, 3, 4, 5);
        final Iterator<Integer> itr = Iterables.cycle(heights).iterator();

        mMutableTestMethod = () -> {
            mText.setHeight(itr.next());
        };
        assertPass("Cycling through increasing heights", () -> {
            mText.setHeight(1);
        });
    }

    private void testSetWidth() {
        final List<Integer> widths = Arrays.asList(1, 2, 3, 4, 5);
        final Iterator<Integer> itr = Iterables.cycle(widths).iterator();

        mMutableTestMethod = () -> {
            mText.setWidth(itr.next());
        };
        assertPass("Cycling through increasing widths", () -> {
            mText.setWidth(3.5f);
        });
    }

    private void testSetColor() {
        final List<Integer> colors = Arrays.asList(Color.BLUE,
                Color.CYAN, Color.MAGENTA, Color.RED);
        final Iterator<Integer> itr = Iterables.cycle(colors).iterator();
        mMutableTestMethod = () -> {
            mText.setColor(itr.next());
        };
        assertPass("Cycling through color", () -> {
            mText.setColor(Color.WHITE);
        });
    }

    private void testSetFontFamily() {
        final List<String> strings = Arrays.asList("Roboto", "DroidSansMono", "CutiveMono", "sans-serif-smallcaps", "monospace", "cursive");
        final Iterator<String> itr = Iterables.cycle(strings).iterator();
        mMutableTestMethod = () -> {
            mText.setFontFamilies(itr.next());
        };
        assertPass("Cycling through font families", () -> {
            mText.setFontFamilies("Roboto");
        });
    }

    private void testSetFontStyle() {
        final List<Text.FontStyle> styles = Arrays.asList(Text.FontStyle.Normal, Text.FontStyle.Italic);
        final Iterator<Text.FontStyle> itr = Iterables.cycle(styles).iterator();
        mMutableTestMethod = () -> {
            mText.setFontStyle(itr.next());
        };
        assertPass("Cycling through font styles", () -> {
            mText.setFontStyle(Text.FontStyle.Normal);
        });
    }

    private void testSetFontWeight() {
        final List<Text.FontWeight> styles = Arrays.asList(Text.FontWeight.values());
        final Iterator<Text.FontWeight> itr = Iterables.cycle(styles).iterator();
        mMutableTestMethod = () -> {
            mText.setFontWeight(itr.next());
        };
        assertPass("Cycling through font weights", () -> {
            mText.setFontWeight(Text.FontWeight.Regular);
        });
    }

    private void testInternationalization() {
        mText.setText("This is an example of mixed text 他们赋 有理性和良心");
        mText.setFontFamilies("Roboto, NotoSansCJK");

        assertPass("Displaying mixed English and Chinese text", () -> {
            mText.setFontFamilies("Roboto");
            mText.setText(TEST_STRING);
        });
    }

    private void testSetText() {
        final List<String> strings = Arrays.asList(new String("Jived fox nymph grabs quick waltz."),
                new String("Glib jocks quiz nymph to vex dwarf."),
                new String("Sphinx of black quartz, judge my vow."),
                new String("How vexingly quick daft zebras jump!"),
                new String("The five boxing wizards jump quickly."),
                new String("Pack my box with five dozen liquor jugs."));
        final Iterator<String> itr = Iterables.cycle(strings).iterator();
        mMutableTestMethod = () -> {
            mText.setText(itr.next());
        };
        assertPass("Cycling through a few pangram strings", () -> {
            mText.setText(TEST_STRING);
        });
    }

    private void testSetFontSize() {
        mMutableTestMethod = () -> {
            mText.setFontSize(mText.getFontSize() + 1);
        };
        assertPass("+1 fontSize per second", () -> {
            mText.setFontSize(25);
        });
    }

    private void testExtrusion() {
        Material frontMaterial = new Material();
        frontMaterial.setDiffuseColor(Color.BLUE);
        Material backMaterial = new Material();
        backMaterial.setDiffuseColor(Color.BLUE);
        mText.setMaterials(Arrays.asList(frontMaterial, backMaterial));

        mText.setExtrusionDepth(8);

        final List<String> strings = Arrays.asList(new String("Jived fox nymph grabs quick waltz."),
                new String("Glib jocks quiz nymph to vex dwarf."),
                new String("Sphinx of black quartz, judge my vow."),
                new String("How vexingly quick daft zebras jump!"),
                new String("The five boxing wizards jump quickly."),
                new String("Pack my box with five dozen liquor jugs."));
        final Iterator<String> itr = Iterables.cycle(strings).iterator();
        mMutableTestMethod = () -> {
            mText.setText(itr.next());

        };
        assertPass("Cycling through strings of 3D blue text");
    }

    private void testExtrusionColors() {
        mText.setExtrusionDepth(8);

        Material frontMaterial = new Material();
        frontMaterial.setDiffuseColor(Color.WHITE);
        Material  backMaterial = new Material();
        backMaterial.setDiffuseColor(Color.BLUE);
        Material sideMaterial = new Material();
        sideMaterial.setDiffuseColor(Color.RED);

        List<Material> materials = Arrays.asList(frontMaterial, backMaterial, sideMaterial);
        mText.setMaterials(materials);

        final List<String> strings = Arrays.asList(new String("Jived fox nymph grabs quick waltz."),
                new String("Glib jocks quiz nymph to vex dwarf."),
                new String("Sphinx of black quartz, judge my vow."),
                new String("How vexingly quick daft zebras jump!"),
                new String("The five boxing wizards jump quickly."),
                new String("Pack my box with five dozen liquor jugs."));
        final Iterator<String> itr = Iterables.cycle(strings).iterator();
        mMutableTestMethod = () -> {
            mText.setText(itr.next());
        };
        assertPass("Cycling through strings of 3D white text with red sides");
    }

    private void testExtrusionAltering() {
        mText.setText("Extrusion Test");
        final List<Float> extrusion = Arrays.asList(new Float(0), new Float(4),
                new Float(8), new Float(16), new Float(32), new Float(64));

        final Iterator<Float> itr = Iterables.cycle(extrusion).iterator();
        mMutableTestMethod = () -> {
            mText.setExtrusionDepth(itr.next());
        };
        assertPass("Increasing extrusion depth");
    }

    private void testOutline() {
        mText.setOuterStroke(Text.OuterStroke.OUTLINE, 2, Color.BLUE);
        assertPass("Text with blue outline");
    }

    private void testDropShadow() {
        mText.setOuterStroke(Text.OuterStroke.DROP_SHADOW, 2, Color.RED);
        assertPass("Text with red drop shadow");
    }

    private void testOutlineColorChange() {
        final List<Integer> colors = Arrays.asList(Color.BLUE, Color.RED, Color.MAGENTA, Color.DKGRAY);
        final Iterator<Integer> itr = Iterables.cycle(colors).iterator();
        mMutableTestMethod = () -> {
            mText.setOuterStroke(Text.OuterStroke.OUTLINE, 2, itr.next());
        };
        assertPass("Text with alternating color outline");
    }

    private void testThickOutline() {
        mText.setOuterStroke(Text.OuterStroke.OUTLINE, 4, Color.RED);
        assertPass("Text with thick red outline");
    }

    private void testAlternatingOuterStroke() {
        final List<Text.OuterStroke> strokes = Arrays.asList(Text.OuterStroke.NONE, Text.OuterStroke.OUTLINE, Text.OuterStroke.DROP_SHADOW);

        final Iterator<Text.OuterStroke> itr = Iterables.cycle(strokes).iterator();
        mMutableTestMethod = () -> {
            mText.setOuterStroke(itr.next(), 2, Color.BLUE);
        };
        assertPass("Text with alternating outer stroke (none, outline, drop shadow)");
    }
}
