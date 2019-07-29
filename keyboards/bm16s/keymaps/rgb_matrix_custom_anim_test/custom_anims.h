RGB_MATRIX_EFFECT(REACTIVE_DIFFUSION)
#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#include "reactive_diffusion_anim.h"

bool REACTIVE_DIFFUSION(effect_params_t* params) {
    // Does rendering, but evaluation iteration done in seperate scan
    // function for greater user control
    //RGB_MATRIX_USE_LIMITS(led_min, led_max);
    uint8_t led_min = RGB_MATRIX_LED_PROCESS_LIMIT * params->iter;
    uint8_t led_max = led_min + RGB_MATRIX_LED_PROCESS_LIMIT;

    if (params->init) {
        reactive_diffusion_reset();
        rgb_matrix_set_color_all(0, 0, 0);
    }

    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t row = i / MATRIX_COLS;
        uint8_t col = i % MATRIX_COLS;
        reactive_diffusion_updateled(row, col);
    }

    return led_max < sizeof(rgb_frame_buffer);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
