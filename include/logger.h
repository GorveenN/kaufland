#ifndef UART_H
#define UART_H

// Puts str into the send queue.
void uart_print(const char *str);

// Configures uart.
// f is callback called when character is read from uart
void uart_conf(void (*f)(char));

#endif
