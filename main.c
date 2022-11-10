/*
 * CyBot Conquerors - Autonomous CyRide
 * CPR E 288 Final Project - Iowa State University
 * main.c
 *
 * Created on: Nov 10, 2022
 * Authors: Henry Shires, Tyler Smith, Emile Albert Kum Chi, and Cael O'Reagan
 */

/* CyBot Subsystems */
#include "adc.h"
#include "button.h"
#include "lcd.h"
#include "movement.h"
#include "open_interface.h"
#include "ping.h"
#include "servo.h"
#include "Timer.h"
#include "uart.h"

/* CyBot Helper Functions */
#include "helpers.h"

/* Global Variables and Flags */
volatile char flag;

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

    int robotMode = 0; // Flag to store if robot is in auto mode (1) or manual mode (0)

    /* Program Main Thread */

    // Define the start position and know where it is at all times
    // Define the location of each stop
    //

    while (1)
    {
        char msg = uart_receive();

        // Toggle mode
        if (msg == 't') {
            robotMode = !robotMode;
        }

        // If 0, Manual mode
        if (robotMode == 0) {
            if (msg == 'w')
            {
                oi_setWheels(250, 250);
            }
            else if (msg == 's')
            {
                oi_setWheels(-250, -250);
            }
            else if (msg == 'a')
            {
                oi_setWheels(250, -250);
            }
            else if (msg == 'd')
            {
                oi_setWheels(-250, 250);
            }
            else if (msg == 'm')
            {
                oi_setWheels(0, 0);
                flag = 0;
                detect_obj(); // Run object scan
            }
            else if (msg == 'q')
            {
                break; // Quit and free memory
            }

            if (sensor_data->bumpLeft == 1) { // We hit something!
                oi_setWheels(0, 0);
                timer_waitMillis(300);
                avoid_bump(sensor_data, 0); // Defaults to left if both are 1 since left is checked first
            } else if (sensor_data->bumpRight == 1) {
                oi_setWheels(0, 0);
                timer_waitMillis(300);
                avoid_bump(sensor_data, 1);
            }

            oi_update(sensor_data);

            timer_waitMillis(100);
            oi_setWheels(0, 0);
        } else {
            // Otherwise 1, Auto mode
            flag = 0;
            auto_drive(sensor_data);
        }
    }

    oi_free(sensor_data);
}
