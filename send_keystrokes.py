from serial import Serial
from pynput.keyboard import Key, Controller
import random

buttons_msgs = {
    "LEFT_P": [Key.left, True],
    "RIGHT_P": [Key.right, True],
    "UP_P": [Key.up, True],
    "DOWN_P": [Key.down, True],
    "ACTION_P": [Key.space, True],
    "LEFT_R": [Key.left, False],
    "RIGHT_R": [Key.right, False],
    "UP_R": [Key.up, False],
    "DOWN_R": [Key.down, False],
    "ACTION_R": [Key.space, False],
}


def led_on(ser):
    idx = random.randint(0, 2)
    ser.write(leds_en[idx])
    return leds_dis[idx]


def led_off(ser, off):
    ser.write(off)


leds_en = [b"RE\x00", b"GE\x00", b"BE\x00"]
leds_dis = [b"RD\x00", b"GD\x00", b"BD\x00"]

keyboard = Controller()
with Serial('/dev/rfcomm1', 9600) as ser:
    while True:
        line = ser.readline().decode("ASCII")[:-1]
        key, pressed = buttons_msgs[line]
        if pressed:
            off = led_on(ser)
            keyboard.press(key)
        else:
            led_off(ser, off)
            keyboard.release(key)
        print(line)
