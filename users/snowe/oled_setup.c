/*
 * Copyright QMK Community
 * Copyright 2021 Tyler Thrailkill (@snowe/@snowe2010) <tyler.b.thrailkill@gmail.com>
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

#ifdef OLED_ENABLE

#    include QMK_KEYBOARD_H
#    include "quantum.h"
#    include "snowe.h"

#    include <stdio.h>  // for keylog?

static uint32_t oled_timer = 0;
bool oled_status = true;

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    if (!is_keyboard_master()) {
        return OLED_ROTATION_270;  // flips the display 180 degrees if offhand
    }
    return OLED_ROTATION_270;
}

#    define L_BASE 0
#    define L_GAME 2
#    define L_LOWER 4
#    define L_RAISE 6
#    define L_ADJUST 8

void oled_render_layer_state(void) {
    oled_write_P(PSTR("Layer"), false);
    static const char PROGMEM arrow[2] = {16,'\0'};
    oled_write_P(arrow, false);
    switch (layer_state) {
        case L_BASE:
            oled_write_P(PSTR("Main"), false);
            break;
        case L_GAME:
            oled_write_ln_P(PSTR("Bot"), false);
            break;
        case L_LOWER:
            oled_write_ln_P(PSTR("Top"), false);
            break;
        case L_RAISE:
            oled_write_ln_P(PSTR("Top"), false);
            break;
//        case L_ADJUST:
//        case L_ADJUST | L_LOWER:
//        case L_ADJUST | L_RAISE:
//        case L_ADJUST | L_LOWER | L_RAISE:
//            oled_write_ln_P(PSTR("Comb"), false);
//            break;
    }
}

char keylog_str[24] = {};

const char code_to_name[60] = {
    ' ', ' ', ' ', ' ', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    'R', 'E', 'B', 'T', '_', '-', '=', '[', ']', '\\',
    '#', ';', '\'', '`', ',', '.', '/', ' ', ' ', ' '};

void set_keylog(uint16_t keycode, keyrecord_t *record) {
    char name = ' ';
    if ((keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX) || (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX)) {
        keycode = keycode & 0xFF;
    }
    if (keycode < 60) {
        name = code_to_name[keycode];
    }

    // update keylog
    snprintf(keylog_str, sizeof(keylog_str), "%dx%d, k%2d : %c", record->event.key.row, record->event.key.col, keycode, name);
}

void render_os_symbol(void){
    /* Show Ctrl-Gui Swap options */
    static const char PROGMEM logo[][2][3] = {
        {{0x97, 0x98, 0}, {0xb7, 0xb8, 0}},
        {{0x95, 0x96, 0}, {0xb5, 0xb6, 0}},
    };
    oled_write_P(PSTR("  "), false);
    if (!keymap_config.swap_lctl_lgui) {
        oled_write_P(logo[1][0], false);
        oled_write_P(PSTR("   "), false);
        oled_write_P(logo[1][1], false);
    } else {
        oled_write_P(logo[0][0], false);
        oled_write_P(PSTR("   "), false);
        oled_write_P(logo[0][1], false);
    }
    oled_write_P(PSTR(" "), false);
}
void render_wpm(void) {
    oled_write_P(PSTR("WPM  "), false);
    static const char PROGMEM arrow[2] = {16,'\0'};
    oled_write_P(arrow, false);
    char wpm[6];
    itoa(get_current_wpm(), wpm, 10);
    oled_write_ln(wpm, false);
}

void render_all(void) {
    if (is_keyboard_master()) {
        render_os_symbol();
        oled_write_P(PSTR("     "), false);
        oled_write_P(PSTR("     "), false);
        oled_render_layer_state();
        oled_write_P(PSTR("     "), false);
        oled_write_P(PSTR("     "), false);
        render_wpm();
    #ifdef LUNA_ENABLE
        led_usb_state = host_keyboard_led_state();
        render_luna(0, 13);
    #endif
    } else {
    #ifdef OCEAN_DREAM_ENABLE
        render_stars();
    #endif
    }
}

bool oled_task_user(void) {
    if(get_current_wpm() > 0) {
        oled_timer = timer_read32();
        oled_on();
        oled_status = true;
    }
    if(!oled_status && get_current_wpm() > 0){
        oled_on();
        oled_status = true;
        render_all();
    } else if (!oled_status) {
        oled_off();
    } else {
        if(timer_elapsed32(oled_timer) >= 60000){
            oled_status = false;
            oled_off();
            return false;
        } else {
            oled_on();
            render_all();
        }
    }
    return false;
}

#endif  // OLED_ENABLE
