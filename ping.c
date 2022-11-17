/*
 * ping.c
 *
 *  Created on: Oct 27, 2022
 *      Author: twsmith1
 */

#include "Timer.h"
#include "lcd.h"
#include "ping.h"
#include "uart.h"

int overflowCount; // Global detection of overflow for output

/**
 * Setting up and using the input-edge time mode on the GPTM module
 */
void ping_init(void) {
    // Enable clock on GPIO port B
    SYSCTL_RCGCGPIO_R |= 0b00000010;

    timer_waitMillis(1);

    // Enable Timer 3
    SYSCTL_RCGCTIMER_R |= 0b00001000;

    timer_waitMillis(1);

    // Enable PB3
    GPIO_PORTB_DEN_R |= 0b00001000;

    // Select Timer Function
    GPIO_PORTB_PCTL_R |= 0x00007000;

    /* Configure GPTM Input Edge Time Mode - Using Timer 3B */
    TIMER3_CTL_R &= 0b1111111011111110; // Disable before changes

    TIMER3_CFG_R = 0x4; // Split mode

    TIMER3_TBMR_R = 0b00111; // Count down, capture mode with edge time

    TIMER3_CTL_R |= 0b0000110000000000; // Capture rising and falling edges on B

    TIMER3_TBILR_R = 0xFFFF; // Max value

    TIMER3_TBPR_R = 0xFF; // Preset value

    /* Configure Interrupt */
    TIMER3_IMR_R |= 0b0000011000000000; // Interrupt on timer B
    NVIC_EN1_R = 0x00000010;
    IntRegister(INT_TIMER3B, TIMER3B_Handler); // Bind ISR

    overflowCount = 0; // Reset overflow count
}

/**
 * Send an initial pulse to the PING))) sensor to trigger the send/receive echo signal
 */
void send_pulse(void) {
    // Sets PB3 as output (disable alternate function)
    TIMER3_CTL_R &= 0b1111111011111110; // Disable before changes

    GPIO_PORTB_DIR_R |= 0b00001000;
    GPIO_PORTB_AFSEL_R &= 0b11110111;

    // Set PB3 to high wait 5 microseconds based on PING))) datasheet
    GPIO_PORTB_DATA_R |= 0b00001000;
    timer_waitMicros(5);

    // Set PB3 to low
    GPIO_PORTB_DATA_R &= 0b11110111;

    // Sets PB3 as input to receive pulse back (enable AF)
    GPIO_PORTB_DIR_R &= 0b11110111;
    GPIO_PORTB_AFSEL_R |= 0b00001000; // Enable function to listen to Timer again

    TIMER3_CTL_R |= 0b0000000100000000; // Enable B
}

/**
 * Pulse the PING))) sensor and return the value it receives back
 *
 * Outputs the echo pulse width in clock cycles and milliseconds,
 * the distance to the object in centimeters, and a running count
 * of the number of timer overflows
 *
 */
int ping_read(void) {
    send_pulse();

    // Interrupts to wait for signal
    while (!FLAG);
    int startTime = time;
    FLAG = 0;

    while (!FLAG);
    int endTime = time;
    FLAG = 0;

    int delta;

    // Detect a timer overflow (negative pulse width)
    if (startTime < endTime) {
        // Overflow occurred
        overflowCount++;
        delta = (startTime - 0x000000) + (0xFFFFFF - endTime); // Total time difference between start, when the overflow occurred, and the end (Microcontroller bug means edge cases are 24-bit)

    } else {
        // Normal
        delta = startTime - endTime;
    }

    /* LCD Output */
    double time = ((double)(delta / 2) / 16000000); // Time (ms)
    int dist = time * 34000; // Distance away (cm)
//    lcd_printf("%d clk cycles\nTime: %lf ms\nDistance: %d cm\nOverflows: %d\n", delta, time * 1000, dist, overflowCount);

//    /* Debug: Distance is still accurate with overflow occurrences */
//    if (startTime < endTime) {
//        sprintf(output, "Overflow detected: %d %d\n\r", delta, dist);
//        uart_sendStr(output);
//    }

    return dist;
}

/**
 * Interrupt Handler for Timer 3B
 * Capture PING))) response pulse start and end time (rising and falling edge)
 */
void TIMER3B_Handler(void) {
    if (TIMER3_MIS_R & 0b0000011000000000) {
        time = TIMER3_TBR_R; // Read the sensor
        TIMER3_ICR_R |= 0b0000011000000000; // Interrupt Clear
        FLAG = 1; // Throw flag
    }
}
