/**
 * SKVEC Graphics
 *
 * SKVEC is a simple vector graphics format for the
 * skirmish display driver
 *
 * The uint8_t arrays contain information about the
 * shapes that should be renderer.
 *
 * Bytes:
 *  1: (7th bit, is shape filled) (6th...1st bit shape)
 *  2: x0
 *  3: y0 (upper 8-bit)
 *  4: y0 (lower 8-bit)
 *  5: x1 (width if rect)
 *  6: y1 (upper 8-bit, height if rect)
 *  7: y1 (lower 8-bit, height if rect)
 * --- only if triangle ---
 *  8: x2
 *  9: y2 (upper 8-bit)
 * 10: y2 (lower 8-bit)
 *
 * if the 1st bit is set to 0xff, the renderer stops.
 * It is required that this bit is set at the end
 * of the graphic.
 *
 * The macros X, Y, W, H can be used to simplify
 * the calculation of coordinates (W = X, H = Y).
 * The macro FILL sets the 7th bit of any shape
 * identifier.
 */

#pragma once
#include <stdint.h>

#define Y(y) (((y) >> 8) & 0xff), ((y) & 0xff)
#define X(x) (x)
#define W(x) X(x)
#define H(y) Y(y)

#define LINE 0
#define RECT 1
#define TRIANGLE 2

#define FILL(shape) ((shape) | 0x80)

const uint8_t skvec_genericBorder[] = {
    /*Top Horizontal Rule*/
    FILL(RECT), X(0), Y(0), W(240), H(3),
    /* Left Vertical Rule*/
    FILL(RECT), X(0), Y(0), W(3), H(245),
    /* Right Vertical Rule */
    FILL(RECT), X(237), Y(0), W(3), H(245),
    /*END*/
    0xff
    /**/
};

const uint8_t skvec_borderTypeDefault[] = {
    /* Left Bar */
    FILL(RECT), X(0), Y(0), W(10), H(304),
    /* Right Bar */
    FILL(RECT), X(230), Y(84), W(10), H(220),
    /* Bottom Bar */
    FILL(RECT), X(0), Y(294), W(240), H(10),
    /* Top-Right Triangle */
    FILL(TRIANGLE), X(225), Y(0), X(240), Y(0), X(240), Y(15),
    /* Right-Middle Triangle */
    FILL(TRIANGLE), X(230), Y(84), X(240), Y(74), X(240), Y(86),
    /* Bottom-Right Block */
    FILL(RECT), X(140), Y(279), W(100), H(25),
    /* Bottom-Right Triangle */
    FILL(TRIANGLE), X(200), Y(279), X(230), Y(249), X(230), Y(279),
    /* Bottom-Middle Triangle */
    FILL(TRIANGLE), X(125), Y(294), X(140), Y(279), X(140), Y(294),
    /* Bottom-Left Triangle */
    FILL(TRIANGLE), X(10), Y(274), X(10), Y(294), X(30), Y(294),
    /* Top-Left Block */
    FILL(RECT), X(10), Y(0), W(25), H(20),
    /* Top-Left-Right Triangle */
    FILL(TRIANGLE), X(35), Y(0), X(54), Y(0), X(35), Y(19),
    /* Top-Left-Left Triangle */
    FILL(TRIANGLE), X(10), Y(20), X(15), Y(20), X(10), Y(25),
    /*END*/
    0xff
    /**/
};

const uint8_t skvec_borderTypeGame[] = {
    /* Left Block */
    FILL(RECT), X(3), Y(74), W(15), H(80),
    /* Left-Top Triangle */
    FILL(TRIANGLE), X(2), Y(59), X(2), Y(74), X(17), Y(74),
    /* Left-Bottom Triangle */
    FILL(TRIANGLE), X(2), Y(154), X(17), Y(154), X(2), Y(169),
    /* Right Block */
    FILL(RECT), X(222), Y(94), W(15), H(80),
    /* Right-Top Triangle */
    FILL(TRIANGLE), X(237), Y(79), X(237), Y(94), X(222), Y(94),
    /* Right-Bottom Triangle */
    FILL(TRIANGLE), X(222), Y(174), X(237), Y(174), X(237), Y(189),
    /* END*/
    0xff
    /**/
};

const uint8_t skvec_gameNameBlock[] = {
    /* Top Block */
    FILL(RECT), X(0), Y(0), W(145), H(25),
    /* Bottom Block */
    FILL(RECT), X(15), Y(15), W(145), H(25),
    /* Left Triangle */
    FILL(TRIANGLE), X(0), Y(24), X(15), Y(24), X(15), Y(39),
    /* Right Triangle */
    FILL(TRIANGLE), X(144), Y(0), X(144), Y(15), X(159), Y(15),
    /**/
    0xff
    /**/
};

const uint8_t skvec_bluetooth[] = {
    LINE, X(3), Y(0), X(3), Y(12), LINE, X(0), Y(3), X(6),
    Y(8), LINE, X(6), Y(3), X(0),  Y(8), LINE, X(3), Y(0),
    X(6), Y(3), LINE, X(3), Y(12), X(6), Y(9), 0xff,
};

const uint8_t skvec_btCross[] = {LINE, X(0), Y(14), X(8),  Y(0), LINE,
                                 X(0), Y(0), X(8),  Y(14), 0xff};

const uint8_t skvec_battery[] = {
    /* Outline */
    RECT, X(0), Y(0), W(20), H(10),
    /* Contact */
    RECT, X(20), Y(2), W(2), H(6),
    /**/
    0xff
    /**/
};
