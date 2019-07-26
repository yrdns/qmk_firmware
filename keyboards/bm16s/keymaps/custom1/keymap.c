#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[0] = LAYOUT_ortho_4x4(
		KC_ESC,  KC_MPLY, MO(3),   MO(1),   \
		KC_F1,   KC_F2,   KC_F3,   KC_F4,   \
		KC_F5,   KC_F6,   KC_F7,   KC_F8,   \
		KC_F9,   KC_F10,  KC_F11,  KC_F12   \
	),
    [1] = LAYOUT_ortho_4x4(
        _______, _______, MO(2),   _______, \
        KC_MPRV, KC_UP,   KC_MNXT, KC_VOLU, \
        KC_LEFT, KC_DOWN, KC_RGHT, KC_VOLD, \
        XXXXXXX, XXXXXXX, XXXXXXX, KC_MUTE  \
    ),
    [2] = LAYOUT_ortho_4x4(
        RESET,   _______, _______, _______, \
        _______, KC_PGUP, _______, _______, \
        _______, KC_DOWN, _______, _______, \
        _______, _______, _______, _______  \
    ),
	[3] = LAYOUT_ortho_4x4(
		_______, _______, _______, MO(4),   \
		RGB_TOG, RGB_M_B, RGB_M_SW,RGB_M_P, \
		RGB_HUI, RGB_SAI, RGB_VAI, RGB_MOD, \
		RGB_HUD, RGB_SAD, RGB_VAD, RGB_RMOD \
	),
    [4] = LAYOUT_ortho_4x4(
        RESET,   _______, _______, _______, \
        RGB_M_T, _______, _______, _______, \
        _______, _______, _______, _______, \
        _______, _______, _______, _______  \
    ),
};

