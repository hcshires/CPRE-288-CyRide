/*
 * ping.h
 *
 *  Created on: Oct 27, 2022
 *      Author: twsmith1
 *
 *  Various functions to configure and use the PING))) sensor via the GPTM module on the CyBot
 */

#ifndef PING_H_
#define PING_H_

#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"

extern volatile char FLAG;
volatile int  time;

/**
 * Setting up and using the input-edge time mode on the GPTM module
 */
void ping_init(void);

/**
 * Send an initial pulse to the PING))) sensor to trigger the send/receive echo signal
 */
void send_pulse(void);

/**
 * Pulse the PING))) sensor and return the value it receives back
 *
 */
int ping_read(void);

/**
 * Interrupt Handler for Timer 3B
 * Capture PING))) response pulse start and end time (rising and falling edge)
 */
void TIMER3B_Handler(void);

#endif /* PING_H_ */
