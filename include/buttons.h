#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>

typedef enum {
    LEFT_JOYSTICK = 0,
    RIGHT_JOYSTICK = 1,
    UP_JOYSTICK = 2,
    DOWN_JOYSTICK = 3,
    ACTION_JOYSTICK = 4,
    USER = 5,
    ATMODE = 6
} BUTTON_ID;

// Configures button GPIO.
void buttons_conf(void (*f)(BUTTON_ID, bool));

bool buttons_is_pressed(BUTTON_ID);

#endif
