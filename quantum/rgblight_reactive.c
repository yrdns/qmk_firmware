/* Copyright 2019 Kyle Joswiak
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "rgblight_reactive.h"

extern rgblight_config_t rgblight_config;

// Currently assumes atleast 2 rows+cols
#define ISQRT2_8FRAC 0xB5 // 181 ~= 181.0193
#define ISQRT2_16FRAC 0xB505 // 46341 ~= 46340.95

static inline fract8 round16to8(fract16 f) {
    return (f&0xFF) + (f&1) > 0x80 ? (f>>8) + 1 : f>>8; // Round to even
}

fract8 reactive_decay = REACTIVE_DECAY;
#ifdef RGBLIGHT_REACTIVE_DIFFUSE_ON
fract8 reactive_diffuse = REACTIVE_DIFFUSE;
uint8_t reactive_evaldir;
#endif
fract8 reactive_acc[REACTIVE_ROWS][REACTIVE_COLS];

void rgblight_reactive_recordpress(keyevent_t *event) {
    if (rgblight_config.enable && rgblight_config.mode ==
                                  RGBLIGHT_MODE_REACTIVE) {
        uint8_t y = event->key.row + REACTIVE_EDGE_BLEED;
        uint8_t x = event->key.col + REACTIVE_EDGE_BLEED;

        reactive_acc[y][x] = 0xFF;
        rgblight_reactive_updatelights();
    }
}

void rgblight_reactive_reset(void) {
    for (uint8_t j=0; j<REACTIVE_ROWS; j++)
        for (uint8_t i=0; i<REACTIVE_COLS; i++)
            reactive_acc[j][i] = 0;
}

void rgblight_reactive_updatespeed(void) {
    uint8_t speed = rgblight_config.speed;
    fract16 decay = (0x0100 - REACTIVE_DECAY)<<8;
#ifdef RGBLIGHT_REACTIVE_DIFFUSE_ON
    fract16 diffuse = (0x0100 - REACTIVE_DIFFUSE)<<8;
#endif
    for (uint8_t i=1; i<speed; i++) {
        if (REACTIVE_DECAY) decay = scale16(decay, REACTIVE_DECAY);
#ifdef RGBLIGHT_REACTIVE_DIFFUSE_ON
        if (REACTIVE_DIFFUSE) diffuse = scale16(diffuse, REACTIVE_DIFFUSE);
#endif
    }
    if (REACTIVE_DECAY) reactive_decay = round16to8(~decay + 1);
#ifdef RGBLIGHT_REACTIVE_DIFFUSE_ON
    if (REACTIVE_DIFFUSE) reactive_diffuse = round16to8(~diffuse + 1);
#endif
}

void rgblight_reactive_update(animation_status_t *anim) {
    if (REACTIVE_DECAY) {
        // Do decay
        for (uint8_t j = 0; j < REACTIVE_ROWS; j++)
            for (uint8_t i = 0; i < REACTIVE_COLS; i++)
                reactive_acc[j][i] = blend8(reactive_acc[j][i], 0,
                                            reactive_decay);
    }

#if defined(RGBLIGHT_REACTIVE_DIFFUSE_ON) || \
    defined(RGBLIGHT_REACTIVE_WAVE_ON)
    for (uint8_t j = 0; j < REACTIVE_ROWS; j++)
        for (uint8_t i = 0; i < REACTIVE_COLS; i++)
            rgblight_reactive_updatecell(i, j);
    reactive_evaldir = !reactive_evaldir;
#endif

    rgblight_reactive_updatelights();
}

void rgblight_reactive_updatecell(uint8_t x, uint8_t y) {
#if defined(RGBLIGHT_REACTIVE_DIFFUSE_ON) || \
    defined(RGBLIGHT_REACTIVE_WAVE_ON)
    uint8_t px, nx, py;
    bool pxspread, nxspread, pyspread;
    pxspread = x > 0;
    nxspread = x < REACTIVE_COLS-1;
    pyspread = y > 0;

    // Usese a swapping eval direction for numeric stability
    if (reactive_evaldir) {
        px = x-1;
        nx = x+1;
        py = y-1;
    } else {
        x = REACTIVE_COLS-1-x;
        y = REACTIVE_ROWS-1-y;
        px = x+1;
        nx = x-1;
        py = y+1;
    }
    uint8_t op = reactive_acc[y][x];
    uint8_t np = op;
    int16_t d;

    if (pxspread) {
        d = op - lerp8by8(op, reactive_acc[y][px], reactive_diffuse);
        reactive_acc[y][px] += d;
        np -= d;
    }
    if (pyspread) {
        d = op - lerp8by8(op, reactive_acc[py][x], reactive_diffuse);
        reactive_acc[py][x] += d;
        np -= d;

        uint8_t corner_diffuse = scale8(reactive_diffuse, ISQRT2_8FRAC);
        if (pxspread) {
            d = op - lerp8by8(op, reactive_acc[py][px], corner_diffuse);
            reactive_acc[py][px] += d;
            np -= d;
        }
        if (nxspread) {
            d = op - lerp8by8(op, reactive_acc[py][nx], corner_diffuse);
            reactive_acc[py][nx] += d;
            np -= d;
        }
    }
    reactive_acc[y][x] = np;
#endif
}

void rgblight_reactive_updatelights(void) {
    for (uint8_t j=0; j < MATRIX_ROWS; j++) {
        uint8_t y = j + REACTIVE_EDGE_BLEED;
        for (uint8_t i=0; i<MATRIX_COLS; i++) {
            uint8_t x = i + REACTIVE_EDGE_BLEED;
            set_rgb_mag(j, i, reactive_acc[y][x]);
        }
    }
    rgblight_set();
}

void set_rgb_mag(uint8_t row, uint8_t col, uint8_t mag) {
    uint16_t mag16 = REACTIVE_INITIAL_MAG*(uint32_t)mag;
    mag = mag16 >= 0xFF ? 0xFF : (uint8_t)mag16;

    sethsv(rgblight_config.hue,
           rgblight_config.sat,
           blend8(0, rgblight_config.val, mag),
           (LED_TYPE *)&led[row*MATRIX_COLS + col]);
}

