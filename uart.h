/*
 * uart.h
 * Custom library for implementing the UART module on the CyBot
 * Used to set up the UART [UART1 at 115200]
 */

#ifndef UART_H_
#define UART_H_

#include "open_interface.h"

#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"

// These two varbles have been declared
// in the file containing main
extern volatile  char uart_data;  // Your UART interrupt code can place read data here
extern volatile  char STOP_FLAG;
extern volatile  char STOP_FLAG_2;
extern volatile  char STOP_FLAG_3;

/**
 * Initialize the UART module
 */
void uart_init(int baud);

/**
 * Send a character to the Serial terminal (PuTTY)
 */
void uart_sendChar(char data);

/**
 * Receive a character from the Serial terminal (from PuTTY)
 */
char uart_receive(void);

/**
 * Send a string over UART (multiple character input)
 */
void uart_sendStr(const char *data);

/**
 * Initialize the UART interrupt
 */
void uart_interrupt_init();

/**
 * Interrupt Service Routine for UART
 */
void uart_interrupt_handler();

#endif /* UART_H_ */
