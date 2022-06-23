RGB_MATRIX_EFFECT(GRADIENT_UP_DOWN_REACTIVE)
RGB_MATRIX_EFFECT(GRADIENT_UP_DOWN_REACTIVE_SIMPLE)
RGB_MATRIX_EFFECT(GRADIENT_UP_DOWN_REACTIVE_NEXUS)
RGB_MATRIX_EFFECT(GRADIENT_UP_DOWN_REACTIVE_NEXUS_INVERSE)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// Math effect functions
// =====================
// Math for when you click getting the opposite color for that key
static HSV GRADIENT_UP_DOWN_REACTIVE_math(HSV hsv, uint16_t offset) {
    hsv.h += qsub8(130, offset);
    return hsv;
}

// WIP: Math for setting the brightness. When you click the keypressed goes black and fades back to your light level.
static HSV GRADIENT_UP_DOWN_REACTIVE_SIMPLE_math(HSV hsv, uint16_t offset) {
    if(offset > rgb_matrix_config.hsv.v){
        offset = rgb_matrix_config.hsv.v;
    }
    hsv.v = scale8(offset, hsv.v);
    return hsv;
}

// Starts with no light level applies the nexus effect.
static HSV GRADIENT_UP_DOWN_REACTIVE_NEXUS_math(HSV hsv, int16_t dx, int16_t dy, uint8_t dist, uint16_t tick) {
    uint16_t effect = tick - dist;
    if (effect > 255) effect = 255;
    if (dist > 72) effect = 255;
    if ((dx > 8 || dx < -8) && (dy > 8 || dy < -8)) effect = 255;
    hsv.v = qadd8(hsv.v, 255 - effect);
    hsv.h = hsv.h + dy / 4;
    return hsv;
}

// WIP Inverse of the above. Lights start on, but when clicked turn off the lights around it. Needs tweaking
static HSV GRADIENT_UP_DOWN_REACTIVE_NEXUS_INVERSE_math(HSV hsv, int16_t dx, int16_t dy, uint8_t dist, uint16_t tick) {
    uint16_t effect = tick - dist;
    if (effect > rgb_matrix_config.hsv.v) effect = rgb_matrix_config.hsv.v;
    if (dist > 72) effect = rgb_matrix_config.hsv.v;
    if ((dx > 8 || dx < -8) && (dy > 8 || dy < -8)) effect = rgb_matrix_config.hsv.v;
     hsv.v = qadd8(0, effect);
    hsv.h = hsv.h + dy / 4;
    return hsv;
}


//  Animation functions
// ======================
// Applies the gradient up down animation effect and then calls the button press with offset.
static bool GRADIENT_UP_DOWN_REACTIVE(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    HSV     hsv   = rgb_matrix_config.hsv;
    uint16_t max_tick = 65535 / rgb_matrix_config.speed;
    uint8_t scale = scale8(64, rgb_matrix_config.speed);
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint16_t tick = max_tick;

        for (int8_t j = g_last_hit_tracker.count - 1; j >= 0; j--) {
            if (g_last_hit_tracker.index[j] == i && g_last_hit_tracker.tick[j] < tick) {
                tick = g_last_hit_tracker.tick[j];
                break;
            }
        }
        // The y range will be 0..64, map this to 0..4
        // Relies on hue being 8-bit and wrapping
        hsv.h   = rgb_matrix_config.hsv.h + scale * (g_led_config.point[i].y >> 4);

        //set offest based on the gradient color.
        uint16_t offset = scale16by8(tick, rgb_matrix_config.speed);
        RGB      rgb    = rgb_matrix_hsv_to_rgb(GRADIENT_UP_DOWN_REACTIVE_math(hsv, offset));
        // RGB rgb = rgb_matrix_hsv_to_rgb(hsv);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return led_max < DRIVER_LED_TOTAL;
}

// Applies the gradient up down effect and then calls for changing the V
static bool GRADIENT_UP_DOWN_REACTIVE_SIMPLE(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    HSV     hsv   = rgb_matrix_config.hsv;
    uint16_t max_tick = 65535 / rgb_matrix_config.speed;
    uint8_t scale = scale8(64, rgb_matrix_config.speed);
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint16_t tick = max_tick;

        for (int8_t j = g_last_hit_tracker.count - 1; j >= 0; j--) {
            if (g_last_hit_tracker.index[j] == i && g_last_hit_tracker.tick[j] < tick) {
                tick = g_last_hit_tracker.tick[j];
                break;
            }
        }
        // The y range will be 0..64, map this to 0..4
        // Relies on hue being 8-bit and wrapping
        hsv.h   = rgb_matrix_config.hsv.h + scale * (g_led_config.point[i].y >> 4);

        //set offest based on the gradient color.
        uint16_t offset = scale16by8(tick, rgb_matrix_config.speed);
        RGB      rgb    = rgb_matrix_hsv_to_rgb(GRADIENT_UP_DOWN_REACTIVE_SIMPLE_math(hsv, offset));
        // RGB rgb = rgb_matrix_hsv_to_rgb(hsv);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return led_max < DRIVER_LED_TOTAL;
}

// Applies the gradient up down effect then calls to get the nexus pattern
static bool GRADIENT_UP_DOWN_REACTIVE_NEXUS(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    HSV     hsv   = rgb_matrix_config.hsv;
    // uint16_t max_tick = 65535 / rgb_matrix_config.speed;
    uint8_t count = g_last_hit_tracker.count;
    uint8_t scale = scale8(64, rgb_matrix_config.speed);
    uint8_t start = qsub8(g_last_hit_tracker.count, 1);
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        hsv.v   = 0;
        for (uint8_t j = start; j < count; j++) {
            int16_t  dx   = g_led_config.point[i].x - g_last_hit_tracker.x[j];
            int16_t  dy   = g_led_config.point[i].y - g_last_hit_tracker.y[j];
            uint8_t  dist = sqrt16(dx * dx + dy * dy);
            uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], rgb_matrix_config.speed);
            hsv           = GRADIENT_UP_DOWN_REACTIVE_NEXUS_math(hsv, dx, dy, dist, tick);
        }
        hsv.v   = scale8(hsv.v, rgb_matrix_config.hsv.v);
        // The y range will be 0..64, map this to 0..4
        // Relies on hue being 8-bit and wrapping
        hsv.h   = rgb_matrix_config.hsv.h + scale * (g_led_config.point[i].y >> 4);

        RGB rgb = rgb_matrix_hsv_to_rgb(hsv);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return led_max < DRIVER_LED_TOTAL;
}

// Does the inverse of the above. Lights dont darken long enough
// Observed lights turning off after some time. NEEDS WORK.
static bool GRADIENT_UP_DOWN_REACTIVE_NEXUS_INVERSE(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    HSV     hsv   = rgb_matrix_config.hsv;
    // uint16_t max_tick = 65535 / rgb_matrix_config.speed;
    uint8_t count = g_last_hit_tracker.count;
    uint8_t scale = scale8(64, rgb_matrix_config.speed);
    uint8_t start = qsub8(g_last_hit_tracker.count, 1);
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        // hsv.v   = 0;
        for (uint8_t j = start; j < count; j++) {
            int16_t  dx   = g_led_config.point[i].x - g_last_hit_tracker.x[j];
            int16_t  dy   = g_led_config.point[i].y - g_last_hit_tracker.y[j];
            uint8_t  dist = sqrt16(dx * dx + dy * dy);
            uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], rgb_matrix_config.speed);
            hsv           = GRADIENT_UP_DOWN_REACTIVE_NEXUS_INVERSE_math(hsv, dx, dy, dist, tick);
        }
        hsv.v   = scale8(hsv.v, rgb_matrix_config.hsv.v);
        // The y range will be 0..64, map this to 0..4
        // Relies on hue being 8-bit and wrapping
        hsv.h   = rgb_matrix_config.hsv.h + scale * (g_led_config.point[i].y >> 4);

        RGB rgb = rgb_matrix_hsv_to_rgb(hsv);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return led_max < DRIVER_LED_TOTAL;
}

#endif      // RGB_MATRIX_CUSTOM_EFFECT_IMPLS