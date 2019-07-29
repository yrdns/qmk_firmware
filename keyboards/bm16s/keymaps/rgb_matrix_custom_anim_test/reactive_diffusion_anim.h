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
#pragma once

#include "action.h"

#ifndef REACTIVE_DIFFUSION_EVAL_FREQ
#define REACTIVE_DIFFUSION_EVAL_FREQ 50
#endif

#ifndef REACTIVE_DIFFUSION_DECAY
#define REACTIVE_DIFFUSION_DECAY 0x06
#endif

#ifndef REACTIVE_DIFFUSION_STRENGTH
#define REACTIVE_DIFFUSION_STRENGTH 0x04
#endif

#ifndef REACTIVE_DIFFUSION_INITIAL_MAG
#define REACTIVE_DIFFUSION_INITIAL_MAG 0x00
#endif

#ifndef REACTIVE_DIFFUSION_HELD_MAG
#define REACTIVE_DIFFUSION_HELD_MAG 0x20
#endif

#ifndef REACTIVE_DIFFUSION_VAL_SCALE
#define REACTIVE_DIFFUSION_VAL_SCALE 4
#endif

bool reactive_diffusion_update(uint8_t iter_min, uint8_t iter_max);
void reactive_diffusion_recordkeypress(keyrecord_t *record);

void reactive_diffusion_reset(void);
void reactive_diffusion_updateled(uint8_t row, uint8_t col);

