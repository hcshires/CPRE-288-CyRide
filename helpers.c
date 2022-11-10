/**
 * helpers.c
 *
 * Functions to utilize various subsystems to accomplish tasks
 * Detect objects, move autonomously, etc.
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

/* C Libraries */
#include <math.h>

// Typedef struct - Obstacle properties such as angle located, distance away from robot, width, and linear width
typedef struct Obstacle
{
    int startAngle;
    int endAngle;
    int angle; // Angle at the midpoint of object
    int startDist;
    int endDist;
    int dist;
    int ping;
    int width;
    int linearWidth;
} Obstacle;

volatile char flag; // Your UART interrupt can update this flag
int STOP_DIST;

/**
 * Output the distance in centimeters the CyBot is away from an object using the onboard IR sensor
 * @returns the number of cm away from an object
 */
int measureDistIR(int raw_val)
{
    return 51792 * pow(raw_val, -1.149);
}

/**
 * Scan the environment for objects close to the Cybot using the front-facing servo IR and ultrasonic sensor
 * @returns the object with the smallest width found
 */
Obstacle detect_obj()
{
    int distancesIR[90];   // Record sensor distancesd

    int curAngle = 0;      // Angle the sensor is currently set at
    int i = 0;             // For loop counter
    int j = 0;            // Counter to keep track of position in obstacles list
    int k = 0;             // Counter for multiple scans
    int distCount = 0;     // Record of how many distances are above normal
    int numObs = 0;        // Object count

    Obstacle obs[7]; // List to record found obstacles
    char output[60]; // Objects output to give PuTTY

    servo_to_right(); // Return to 0 first
    timer_waitMillis(300);

    // Scan and record sensor data
    while (curAngle <= 180)
    {
        if (flag) { break; } // Interrupt

        distancesIR[i] = adc_read(); // Should be averaged value via hardware

        sprintf(output, "%d\t\t%d\n\r", curAngle, measureDistIR(distancesIR[i]));
        uart_sendStr(output);

        servo_move(2);
        curAngle += 2;
        i++;
    }

    // Iterate and find object properties
    for (i = 0; i < 90; i++)
    {
        if (measureDistIR(distancesIR[i]) < 50)
        { // Max distance IR can read is 50 cm
            distCount++;
        }
        else
        {
            if (distCount >= 5)
            {
                Obstacle tempObj;
                tempObj.startAngle = (i - distCount) * 2;
                tempObj.endAngle = (i - 1) * 2;
                tempObj.angle = (tempObj.startAngle + tempObj.endAngle) / 2; // Angle midpoint
                tempObj.startDist = measureDistIR(distancesIR[(i - distCount)]);
                tempObj.endDist = measureDistIR(distancesIR[(i - 1)]);
                tempObj.dist = (tempObj.startDist + tempObj.endDist) / 2; // Dist midpoint
                tempObj.width = tempObj.endAngle - tempObj.startAngle;
                tempObj.linearWidth =
                        sqrt((pow(tempObj.startDist, 2) + pow(tempObj.endDist, 2))
                                - (2 * tempObj.startDist * tempObj.endDist * cos(tempObj.width))); // sqrt(a^2 + b^2 - 2abcos(angle)) = c

                obs[j] = tempObj;
                j++;
                numObs++;
            }

            distCount = 0;
        }
    }

    // Second scan with PING
    for (k = 0; k < numObs; k++)
    {
        servo_to_right();
        servo_move(obs[k].angle);

        obs[k].ping = ping_read();  // Get PING at midpoint
        timer_waitMillis(100);
    }

    // Output list of Objects found
    sprintf(output, "%-12s%-12s%-12s%-12s%-12s%-12s\n\r", "Object", "Angle", "PING", "IR Distance", "Width", "Linear");
    uart_sendStr(output);
    lcd_printf("%d", numObs); // Debug # objects

    int minWidth = obs[0].width; // Object with smallest width
    int smallIndex = 0; // Index of smallest object

    // Find min width
    for (i = 0; i < numObs; i++)
    {
        if (obs[i].width < minWidth)
        {
            minWidth = obs[i].width;
            smallIndex = i;
        }

        // Output list of objects as well
        sprintf(output,
                "%-12d%-12d%-12d%-12d%-12d%-12d\n\r",
                i + 1, obs[i].startAngle, obs[i].ping, obs[i].dist,
                obs[i].width, obs[i].linearWidth);
        uart_sendStr(output);
    }

    if (numObs != 0)
    {
        servo_to_right();
        servo_move(obs[smallIndex].angle); // Point to smallest object

        sprintf(output,
                "Object %d: Drive %d cm at angle %d to reach the smallest object.\n\n\r",
                smallIndex + 1, obs[smallIndex].dist, obs[smallIndex].startAngle);
        uart_sendStr(output); // Output to PuTTY
    }

    return obs[smallIndex];
}

/**
 * Move CyBot autonomously based on the location of the smallest object passed in
 */
void auto_drive(oi_t *sensor_data)
{
    int bumped = 1; // Flag to see if move was interrupted by bump sensor
    Obstacle smallObj;

    while (!flag && bumped) { // Do auto until override or hit target <5 cm
        smallObj = detect_obj(); // Scan again to verify distance, bump sensor may have occured preemptively

        if (smallObj.dist > 1000) { // No objects found?
            move_forward_bump(sensor_data, 150);
            bumped = 1; // Need to scan again
        } else {
            //move_forward_bump(sensor_data, 150); // Match bot's center with angle the sensor detected (15 cm between center and servo)

            if (smallObj.angle < 90) {
                turn_clockwise(sensor_data, (75 - smallObj.angle));

                if (smallObj.angle < 65) {
                    STOP_DIST = -100;
                }
            } else if (smallObj.angle > 90) { // If angle is exactly 90, don't worry about turning
                turn_counterclockwise(sensor_data, (smallObj.angle - 75));

                if (smallObj.angle > 115) {
                    STOP_DIST = -100;
                }
            }
                STOP_DIST = 100;

            bumped = move_forward_bump_auto(sensor_data, (smallObj.dist * 10) - STOP_DIST, smallObj.angle);
        }
    }
}
