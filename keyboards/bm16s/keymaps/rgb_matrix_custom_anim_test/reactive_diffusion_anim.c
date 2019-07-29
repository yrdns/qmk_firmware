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
#include "reactive_diffusion_anim.h"

#include "matrix.h"
#include "rgb_matrix.h"
#include "lib/lib8tion/lib8tion.h"

#define ISQRT2_8FRAC 0xB5

void reactive_diffusion_updatecell(uint8_t row, uint8_t col);

static uint16_t eval_timer = 0;
static bool eval_dir = 0;

void reactive_diffusion_recordkeypress(keyrecord_t *record) {
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;

    if (REACTIVE_DIFFUSION_INITIAL_MAG) {
        rgb_frame_buffer[row][col] = qadd8(rgb_frame_buffer[row][col],
                                           REACTIVE_DIFFUSION_INITIAL_MAG);
    }

    reactive_diffusion_updateled(row, col);
}

void reactive_diffusion_scan() {
    if (rgb_matrix_get_mode() == RGB_MATRIX_CUSTOM_REACTIVE_DIFFUSION) {
        uint16_t ctime = timer_read();
        if (ctime - eval_timer >= REACTIVE_DIFFUSION_EVAL_FREQ) {
            eval_timer = ctime;
            reactive_diffusion_update();
        }
    }
}

void reactive_diffusion_reset(void) {
    memset(rgb_frame_buffer, 0, sizeof(rgb_frame_buffer));
}

void reactive_diffusion_update() {
    for (uint8_t j = 0; j < MATRIX_ROWS; j++) {
        for (uint8_t i = 0; i < MATRIX_COLS; i++) {
            // Do decay
            if (REACTIVE_DIFFUSION_DECAY) {
                rgb_frame_buffer[j][i] = blend8(rgb_frame_buffer[j][i], 0,
                                                REACTIVE_DIFFUSION_DECAY);
            }
            // Scan for held values
            if (REACTIVE_DIFFUSION_HELD_MAG) {
                if (matrix_is_on(j, i)) {
                    rgb_frame_buffer[j][i] = qadd8(rgb_frame_buffer[j][i],
                                             REACTIVE_DIFFUSION_HELD_MAG);
                }
            }
        }
    }

    if (REACTIVE_DIFFUSION_STRENGTH) {
        // Do diffusion
        for (uint8_t j = 0; j < MATRIX_ROWS; j++) {
            for (uint8_t i = 0; i < MATRIX_COLS; i++) {
                reactive_diffusion_updatecell(j, i);
            }
        }
    }

    eval_dir = !eval_dir;
}

void reactive_diffusion_updatecell(uint8_t row, uint8_t col) {
    if (REACTIVE_DIFFUSION_STRENGTH) {
        uint8_t pcol, ncol, prow;
        bool pcolspread, ncolspread, prowspread;
        pcolspread = col > 0;
        ncolspread = col < MATRIX_COLS-1;
        prowspread = row > 0;

        // Usese a swapping eval direction for numeric stability
        if (eval_dir) {
            pcol = col-1;
            ncol = col+1;
            prow = row-1;
        } else {
            col = (MATRIX_COLS-1)-col;
            row = (MATRIX_ROWS-1)-row;
            pcol = col+1;
            ncol = col-1;
            prow = row+1;
        }
        int16_t d;

        uint8_t op = rgb_frame_buffer[row][col];
        uint8_t np = op;
        if (pcolspread) {
            d = op - lerp8by8(op, rgb_frame_buffer[row][pcol],
                              REACTIVE_DIFFUSION_STRENGTH);
            rgb_frame_buffer[row][pcol] += d;
            np -= d;
        }
        if (prowspread) {
            d = op - lerp8by8(op, rgb_frame_buffer[prow][col],
                              REACTIVE_DIFFUSION_STRENGTH);
            rgb_frame_buffer[prow][col] += d;
            np -= d;

            // TODO: Precompute
            uint8_t corner_strength = scale8(REACTIVE_DIFFUSION_STRENGTH,
                                             ISQRT2_8FRAC);
            if (pcolspread) {
                d = op - lerp8by8(op, rgb_frame_buffer[prow][pcol],
                                  corner_strength);
                rgb_frame_buffer[prow][pcol] += d;
                np -= d;
            }
            if (ncolspread) {
                d = op - lerp8by8(op, rgb_frame_buffer[prow][ncol],
                                  corner_strength);
                rgb_frame_buffer[prow][ncol] += d;
                np -= d;
            }
        }
        rgb_frame_buffer[row][col] = np;
    }
}

void reactive_diffusion_updateled(uint8_t row, uint8_t col) {
    uint8_t leds[LED_HITS_TO_REMEMBER];
    uint8_t leds_count = rgb_matrix_map_row_column_to_led(row, col, leds);

    HSV hsv = rgb_matrix_config.hsv;
    hsv.v = scale8(hsv.v, qmul8(rgb_frame_buffer[row][col],
                                REACTIVE_DIFFUSION_VAL_SCALE));
    RGB rgb = hsv_to_rgb(hsv);

    for (uint8_t i = 0; i < leds_count; i++) {
        rgb_matrix_set_color(leds[i], rgb.r, rgb.g, rgb.b);
    }
}
