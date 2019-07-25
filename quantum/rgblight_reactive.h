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
#ifndef RGBLIGHT_REACTIVE_H
#define RGBLIGHT_REACTIVE_H

#include "lib/lib8tion/lib8tion.h"
#include "rgblight.h"
#include "keyboard.h"

#ifndef RGBLIGHT_REACTIVE_INTERVAL
#define RGBLIGHT_REACTIVE_INTERVAL 50
#endif

#ifndef REACTIVE_DECAY
#ifndef RGBLIGHT_REACTIVE_DIFFUSE_ON
  #define REACTIVE_DECAY 0x20
#else
  #define REACTIVE_DECAY 0x03
#endif
#endif

#ifndef REACTIVE_DIFFUSE
#define REACTIVE_DIFFUSE 0x10
#endif

#ifndef REACTIVE_WAVE_STRENGTH
#define REACTIVE_WAVE_STRENGTH 0x1000
#endif

#ifndef REACTIVE_WAVE_DAMPING
#define REACTIVE_WAVE_DAMPING 0xF000
#endif

#ifndef REACTIVE_INITIAL_MAG
#ifndef RGBLIGHT_REACTIVE_DIFFUSE_ON
  #define REACTIVE_INITIAL_MAG 2
#else
  #define REACTIVE_INITIAL_MAG 4
#endif
#endif

#ifndef REACTIVE_EDGE_BLEED
  #if defined(RGBLIGHT_REACTIVE_DIFFUSE_ON) || \
      defined(RGBLIGHT_REACTIVE_WAVE_ON)
    #define REACTIVE_EDGE_BLEED 2
  #else
    #define REACTIVE_EDGE_BLEED 0
  #endif
#endif

#define REACTIVE_ROWS (MATRIX_ROWS + 2*REACTIVE_EDGE_BLEED)
#define REACTIVE_COLS (MATRIX_COLS + 2*REACTIVE_EDGE_BLEED)

void rgblight_reactive_update(animation_status_t *anim);
void rgblight_reactive_recordpress(keyevent_t *event);

void rgblight_reactive_reset(void);
void rgblight_reactive_updatespeed(void);
void rgblight_reactive_updatecell(uint8_t x, uint8_t y);
void rgblight_reactive_updatelights(void);
void set_rgb_mag(uint8_t row, uint8_t col, uint8_t mag);

#endif // RGBLIGHT_REACTIVE_H

