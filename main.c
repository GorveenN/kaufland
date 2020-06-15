#include <bluetooth.h>
#include <buttons.h>
#include <gpio.h>
#include <leds.h>
#include <logger.h>
#include <stm32.h>
#include <string.h>
#include <timer.h>

#define BUTTONS_MSGS_OFFSET 7

#define LF 10
#define CR 13
#define BUFFER_SIZE 9

#define LEDS_MSG_NO 9

const char *buttons_msgs[] = {
    "LEFT_P\n", "RIGHT_P\n",  "UP_P\n",   "DOWN_P\n",  "ACTION_P\n",
    "USER_P\n", "ATMODE_P\n", "LEFT_R\n", "RIGHT_R\n", "UP_R\n",
    "DOWN_R\n", "ACTION_R\n", "USER_R\n", "ATMODE_R\n"};

const char *leds_msgs[] = {"RE", "RD", "RT", "BE", "BD",
                           "BT", "GE", "GD", "GT"};

int8_t get_led_command_idx(const char *buff) {
    for (int i = 0; i < LEDS_MSG_NO; ++i) {
        if (strcmp(buff, leds_msgs[i]) == 0) {
            return i;
        }
    }
    return -1;
}

LED_COLOR get_led_color(int8_t command_array_idx) {
    return command_array_idx / 3;
}

LED_STATE get_led_state(int8_t command_array_idx) {
    return command_array_idx % 3;
}

// called when character arrives over bt/serial with c as received character
void reader_cb(char c) {
    static char buff[BUFFER_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    static uint8_t pos = 0;

    c = c == LF || c == CR || pos == BUFFER_SIZE - 1 ? 0 : c;
    buff[pos++] = c;

    if (c == 0) {
        int8_t cmd_idx = get_led_command_idx(buff);

        if (cmd_idx >= 0) {
            LED_COLOR color = get_led_color(cmd_idx);
            LED_STATE action = get_led_state(cmd_idx);

            switch (action) {
            case ON:
                led_on(color);
                break;
            case OFF:
                led_off(color);
                break;
            case TOGGLE:
                led_toggle(color);
                break;
            }
        }

        for (uint8_t i = 0; i < BUFFER_SIZE; ++i) {
            buff[i] = 0;
        }
        pos = 0;
    }
}

// called after button is pressed/released
void button_cb(BUTTON_ID id, bool pressed) {
    timer_delay(id);
}

// called after timer delay
// should remove button_id from arg and handle events outside
void timer_cb(BUTTON_ID id) {
    const char *str =
        buttons_msgs[id + BUTTONS_MSGS_OFFSET * (1 - buttons_is_pressed(id))];

    bt_print(str);
}

int main() {
    bt_conf(reader_cb);
    timer_conf(timer_cb);
    buttons_conf(button_cb);
    led_conf();
    led_conf();

    while (1)
        ;
    return 0;
}
