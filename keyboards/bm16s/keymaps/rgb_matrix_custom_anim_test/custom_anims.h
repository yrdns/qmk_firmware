RGB_MATRIX_EFFECT(REACTIVE_DIFFUSION)
#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#include "reactive_diffusion_anim.h"

bool REACTIVE_DIFFUSION(effect_params_t* params) {
    uint8_t led_min = RGB_MATRIX_LED_PROCESS_LIMIT * params->iter;
    uint8_t led_max = led_min + RGB_MATRIX_LED_PROCESS_LIMIT;

    if (params->init) {
        reactive_diffusion_reset();
        rgb_matrix_set_color_all(0, 0, 0);
    }

    return reactive_diffusion_update(led_min, led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
