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
#include "matrix.h"

extern rgblight_config_t rgblight_config;

// Currently assumes atleast 2 rows+cols
#define ISQRT2_8FRAC 0xB5 // 181 ~= 181.0193
#define ISQRT2_16FRAC 0xB505 // 46341 ~= 46340.95

static inline fract8 round16to8(fract16 f) {
    // Round to even, unless this would cause overflow
    return (f < 0xFF00 && (f&0xFF) + (f&1) > 0x80 ? (f>>8) + 1 : f>>8;
}

// For some reason lib8tion doesn't have this function, even though signs
//   make it logically distinct from lerp8by8, AND they have lerp15by16
static inline int8_t lerp7by8(int8_t a, int8_t b, fract8 frac) {
    int8_t result;
    if (b > a) {
        uint8_t delta = b - a;
        uint8_t scaled = scale16by8(delta, frac);
        result = a + scaled;
    } else {
        uint8_t delta = a - b;
        uint8_t scaled = scale16by8(delta, frac);
        result = a - scaled;
    }
    return result;
}

uint8_t reactive_decay = REACTIVE_DECAY;

#ifdef RGBLIGHT_REACTIVE_DIFFUSE_ON
uint8_t reactive_diffuse = REACTIVE_DIFFUSE;
uint8_t reactive_evaldir;
#endif

#if defined(RGBLIGHT_REACTIVE_DIFFUSE_ON) || \\
    defined(RGBLIGHT_REACTIVE_WAVE_ON)
uint8_t reactive_acc[REACTIVE_ROWS][REACTIVE_COLS];
#endif
#if defined(RGBLIGHT_REACTIVE_WAVE_ON)
int8_t reactive_vel[REACTIVE_ROWS][REACTIVE_COLS];
#endif

void rgblight_reactive_recordpress(keyevent_t *event) {
    if (rgblight_config.enable && rgblight_config.mode ==
                                  RGBLIGHT_MODE_REACTIVE) {
        uint8_t y = event->key.row + REACTIVE_EDGE_BLEED;
        uint8_t x = event->key.col + REACTIVE_EDGE_BLEED;

        reactive_acc[y][x] = qadd8(reactive_acc[y][x], REACTIVE_INITIAL_MAG);

        rgblight_reactive_updatelights();
    }
}

void rgblight_reactive_reset(void) {
    memset8(reactive_acc, 0, sizeof(reactive_acc));
}

void rgblight_reactive_updatespeed(void) {
    uint8_t speed = rgblight_config.speed;
    uint16_t decay = (0x0100 - REACTIVE_DECAY)<<8;
#ifdef RGBLIGHT_REACTIVE_DIFFUSE_ON
    uint16_t diffuse = (0x0100 - REACTIVE_DIFFUSE)<<8;
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
        for (uint8_t j = 0; j < REACTIVE_ROWS; j++) {
            for (uint8_t i = 0; i < REACTIVE_COLS; i++) {
                reactive_acc[j][i] = blend8(reactive_acc[j][i], 0,
                                            reactive_decay);
            }
        }
    }

    if (REACTIVE_HELD_MAG) {
        // Scan for held values
        for (uint8_t j = 0; j < MATRIX_ROWS; j++) {
            uint8_t y = j + REACTIVE_EDGE_BLEED;
            for (uint8_t i = 0; i < MATRIX_COLS; i++) {
                uint8_t x = i + REACTIVE_EDGE_BLEED;
                if (matrix_is_on(j, i)) {
                    reactive_acc[y][x] = qadd8(reactive_acc[y][x],
                                               REACTIVE_HELD_MAG);
                }
            }
        }
    }

#if defined(RGBLIGHT_REACTIVE_DIFFUSE_ON) || \
    defined(RGBLIGHT_REACTIVE_WAVE_ON)
    for (uint8_t j = 0; j < REACTIVE_ROWS; j++) {
        for (uint8_t i = 0; i < REACTIVE_COLS; i++) {
            rgblight_reactive_updatecell(i, j);
        }
    }
    reactive_evaldir = !reactive_evaldir;
#if defined(RGBLIGHT_REACTIVE_WAVE_ON)
    for (uint8_t j = 0; j < REACTIVE_ROWS; j++) {
        for (uint8_t i = 0; i < REACTIVE_COLS; i++) {
            reactive_acc[y][x]
        }
    }
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
    int16_t d;

#ifdef REACTIVE_DIFFUSE_ON
    uint8_t op = reactive_acc[y][x];
    uint8_t np = op;
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
#ifdef REACTIVE_WAVE_ON
    int8_t ov = reactive_vel[y][x];
    int8_t nv = ov;
    if (pxspread) {
        d = ov - lerp7by8(ov, reactive_vel[y][px], reactive_wave_strength);
        reactive_vel[y][px] += d;
        nv -= d;
    }
    if (pyspread) {
        d = ov - lerp7by8(ov, reactive_vel[py][x], reactive_wave_strength);
        reactive_vel[py][x] += d;
        nv -= d;

        uint8_t corner_strength = scale8(reactive_wave_strength,
                                         ISQRT2_8FRAC);
        if (pxspread) {
            d = ov - lerp7by8(ov, reactive_vel[py][px], corner_strength);
            reactive_vel[py][px] += d;
            nv -= d;
        }
        if (nxspread) {
            d = ov - lerp7by8(ov, reactive_vel[py][nx], corner_strength);
            reactive_vel[py][nx] += d;
            nv -= d;
        }
    }
    reactive_vel[y][x] = nv;
#endif
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
    uint16_t mag16 = REACTIVE_VAL_SCALE*(uint16_t)mag;
    mag = mag16 >= 0xFF ? 0xFF : (uint8_t)mag16;

    sethsv(rgblight_config.hue,
           rgblight_config.sat,
           blend8(0, rgblight_config.val, mag),
           (LED_TYPE *)&led[row*MATRIX_COLS + col]);
}

