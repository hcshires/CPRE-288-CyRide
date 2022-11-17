/*
 * CyBot Conquerors - Autonomous CyRide
 * CPR E 288 Final Project - Iowa State University
 * main.c
 *
 * Created on: Nov 10, 2022
 * Authors: Henry Shires, Tyler Smith, Emile Albert Kum Chi, and Cael O'Reagan
 */

/* CyBot Helper Functions */
#include "helpers.h"

/* Global Variables and Flags */
volatile char FLAG;
volatile char quit; // Stop the program

/**
 * Utilize the CyBot PING))) sensor with a custom library
 */
void main() {
    /* Init CyBot Subsystems */
    adc_init();
    button_init();
    lcd_init();
    ping_init();
    servo_init();
    timer_init();
    uart_init(115200);
    uart_interrupt_init();

    /* iRobot Open Interface */
    oi_t *sensor_data;
    sensor_data = oi_alloc();
    oi_init(sensor_data);

    /* Wait for user to start */
    while (1)
    {
        char msg = uart_receive();

        if (msg == 't') {
            break;
        }
    }

    /* Program Main Thread */
    while (1) {
        FLAG = 0;

        turn_counterclockwise(sensor_data, 75); // Face the passengers
        detect_passengers();
        break;

        // auto_drive(sensor_data);
    }

    oi_free(sensor_data);
}
