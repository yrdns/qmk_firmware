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

#define ISQRT2_8FRAC 0xB5 // 181 ~= 181.0193 ~= 256/sqrt(2)
#define ISQRT2_16FRAC 0xB505

static bool eval_dir = 0;

void reactive_diffusion_recordkeypress(keyrecord_t *record) {
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;

    if (REACTIVE_DIFFUSION_INITIAL_MAG) {
        rgb_frame_buffer[row][col] = qadd8(rgb_frame_buffer[row][col],
                                           REACTIVE_DIFFUSION_INITIAL_MAG);

        reactive_diffusion_updateled(row, col);
    }
}

void reactive_diffusion_reset(void) {
    memset(rgb_frame_buffer, 0, sizeof(rgb_frame_buffer));
}

bool reactive_diffusion_update(uint8_t iter_min, uint8_t iter_max) {
    const uint8_t corner_strength = scale8(REACTIVE_DIFFUSION_STRENGTH,
                                           ISQRT2_8FRAC);

    if (iter_max > MATRIX_ROWS*MATRIX_COLS) {
        iter_max = MATRIX_ROWS*MATRIX_COLS;
    }

    for (uint8_t i = iter_min; i < iter_max; i++) {
        uint8_t row, col, prow, pcol, ncol;
        if (eval_dir) {
            row = i / MATRIX_COLS;
            prow = row > 0 ? row-1 : 0;

            col = i % MATRIX_COLS;
            pcol = col > 0 ? col-1 : 0;
            ncol = col < MATRIX_COLS-1 ? col+1 : MATRIX_COLS-1;
        } else {
            row = ((MATRIX_ROWS*MATRIX_COLS-1) - i) / MATRIX_COLS;
            prow = row < MATRIX_ROWS-1 ? row+1 : MATRIX_ROWS-1;

            col = ((MATRIX_ROWS*MATRIX_COLS-1) - i) % MATRIX_COLS;
            pcol = col < MATRIX_COLS-1 ? col+1 : MATRIX_COLS-1;
            ncol = col > 0 ? col-1 : 0;
        }

        // Do decay
        if (REACTIVE_DIFFUSION_DECAY) {
            rgb_frame_buffer[row][col] =
                blend8(rgb_frame_buffer[row][col], 0,
                       REACTIVE_DIFFUSION_DECAY);
        }
        // Scan for held values
        if (REACTIVE_DIFFUSION_HELD_MAG) {
            if (matrix_is_on(row, col)) {
                rgb_frame_buffer[row][col] =
                    qadd8(rgb_frame_buffer[row][col],
                          REACTIVE_DIFFUSION_HELD_MAG);
            }
        }
        // Do diffusion
        if (REACTIVE_DIFFUSION_STRENGTH) {
            // Important we have swapping eval directions as we pick up the
            //   cummulative diffusion of every cell we've already looked at
            //   in this pass, so if we evaled in the same direction every
            //   time we would create an assymetry bias. Pure iteration would
            //   likely require an additional MATRIX_ROW*MATRIX_COL array to
            //   store the previous state, which isn't worth the space
            //   (OR would atleast require storing the current+last rows)
            int16_t d;

            uint8_t op = rgb_frame_buffer[row][col];
            uint8_t np = op;
            // Left/Right
            d = op - lerp8by8(op, rgb_frame_buffer[row][pcol],
                              REACTIVE_DIFFUSION_STRENGTH);
            rgb_frame_buffer[row][pcol] += d;
            np -= d;
            // Up-left/Down-right
            d = op - lerp8by8(op, rgb_frame_buffer[prow][pcol],
                              corner_strength);
            rgb_frame_buffer[prow][pcol] += d;
            np -= d;
            // Up/Down
            d = op - lerp8by8(op, rgb_frame_buffer[prow][col],
                              REACTIVE_DIFFUSION_STRENGTH);
            rgb_frame_buffer[prow][col] += d;
            np -= d;
            // Up-right/Down-left
            d = op - lerp8by8(op, rgb_frame_buffer[prow][ncol],
                              corner_strength);
            rgb_frame_buffer[prow][ncol] += d;
            np -= d;

            rgb_frame_buffer[row][col] = np;
        }

        // Update led
        reactive_diffusion_updateled(row, col);
    }

    if (iter_max >= MATRIX_ROWS*MATRIX_COLS) {
        eval_dir = !eval_dir;
        return false;
    }
    return true;
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

