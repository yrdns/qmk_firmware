#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[0] = LAYOUT_ortho_4x4(
		XXXXXXX, XXXXXXX, XXXXXXX, MO(1),   \
		XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, \
		XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, \
		XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX  \
    ),
    [1] = LAYOUT_ortho_4x4(
        RESET,   _______, _______, _______, \
        RGB_TOG, RGB_RMOD,RGB_MOD, _______, \
        RGB_HUI, RGB_SAI, RGB_VAI, RGB_SPI, \
        RGB_HUD, RGB_SAD, RGB_VAD, RGB_SPD  \
    ),
};

led_config_t g_led_config = { {
    {  0,  1,  2,  3 },
    {  4,  5,  6,  7 },
    {  8,  9, 10, 11 },
    { 12, 13, 14, 15 }
}, {
    { 0x08, 0x08 }, { 0x18, 0x08 }, { 0x28, 0x08 }, { 0x38, 0x08 },
    { 0x08, 0x18 }, { 0x18, 0x18 }, { 0x28, 0x18 }, { 0x38, 0x18 },
    { 0x08, 0x28 }, { 0x18, 0x28 }, { 0x28, 0x28 }, { 0x38, 0x28 },
    { 0x08, 0x38 }, { 0x18, 0x38 }, { 0x28, 0x38 }, { 0x38, 0x38 }
}, {
    4, 4, 4, 4,
    4, 4, 4, 4,
    4, 4, 4, 4,
    4, 4, 4, 4
} };

