/*
 * CyBot Conquerors - Autonomous CyRide
 * CPR E 288 Final Project - Iowa State University
 * main.c
 *
 * Created on: Nov 10, 2022
 * Authors: Henry Shires, Tyler Smith, Emile Albert Kum Chi, and Cael O'Reagan
 */

/* CyBot Helper Functions */
#include "movement.h"

/**
 * Utilize the CyBot PING))) sensor with a custom library
 *
 * Use CyBot 06
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

    // load_songs(); // OI Songs

    /* Wait for user to start */
    while (1)
    {
        char msg = uart_receive();

        if (msg == 't') {
            break;
        }
    }

    /* Program Main Thread */
    uart_sendStr("Welcome to CyRide! This is #23: Orange Route\n\r");

    turn_clockwise(sensor_data, 80); // Face the passengers, bottom right corner of test field
    int passengerCount = detect_passengers();

    /* Begin the route */
    turn_counterclockwise(sensor_data, 72); // Face the passengers, bottom right corner of test field
    move_forward(sensor_data, 10); // Move forward a bit to add distance to sum
    auto_drive(sensor_data); // Drive the route

    oi_free(sensor_data);
}
