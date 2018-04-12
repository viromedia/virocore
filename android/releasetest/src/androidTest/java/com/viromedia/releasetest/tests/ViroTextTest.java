/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.releasetest.tests;

import android.graphics.Color;
import android.support.test.espresso.core.deps.guava.collect.Iterables;

import com.viro.core.AmbientLight;
import com.viro.core.Node;
import com.viro.core.Text;
import com.viro.core.Vector;

import org.junit.Test;

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

        mText = new Text(mViroView.getViroContext(), TEST_STRING,
                "Roboto", 25, Color.WHITE,
                3.5f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        mTextNode = new Node();
        mTextNode.setPosition(new Vector(0f, 0f, -3.3f));
        mTextNode.setGeometry(mText);
        mScene.getRootNode().addChildNode(mTextNode);
    }

    @Test
    public void testText() {
        testSetText();
        testSetFontFamily();
        testSetFontSize();
        testSetFontStyle();
        testSetFontWeight();
        testInternationalization();
        testSetColor();
        testSetWidth();
        testSetHeight();
        testSetHorizontalAlignment();
        testSetVerticalAlignment();
        testSetLineBreakMode();
        testSetClipMode();
        testSetMaxLines();
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
}
