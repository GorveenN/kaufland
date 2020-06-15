#ifndef BLUETOOTH_H
#define BLUETOOTH_H

// Puts atgument into the send queue.
void bt_print(const char *str);

// Configures uart.
void bt_conf(void (*f)(char));

#endif
