/**
 * helpers.c
 *
 * Functions to utilize various subsystems to accomplish tasks
 * Detect objects, move autonomously, etc.
 */

#include "helpers.h"

/* CyBot Properties */
int NUM_PASSENGERS = 0;
int DETECTED_OBJS = 0;
Obstacle OBJECTS[7];  // List to record found obstacles
char DEBUG_OUTPUT[65]; // Output message to give PuTTY

/* Global Flags */
volatile  char STOP_FLAG;

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
    int distancesIR[91];  // Record sensor distances

    int curAngle = 0;     // Angle the sensor is currently set at
    int i = 0;            // For loop counter
    int distCount = 0;    // Record of how many distances are above normal
    int numObs = 0;       // Object count

    servo_to_right(); // Return to 0 first
    timer_waitMillis(300);

    // Scan and record sensor data
    while (curAngle <= 180)
    {
        distancesIR[i] = adc_read(); // Should be averaged value via hardware

        //sprintf(DEBUG_OUTPUT, "%d\t\t%d\n\r", curAngle, measureDistIR(distancesIR[i]));
        //uart_sendStr(DEBUG_OUTPUT);

        servo_move(2);
        curAngle += 2;
        i++;
    }

    // Iterate and find object properties
    for (i = 0; i < 91; i++)
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
                tempObj.linearWidth = tempObj.width * (M_PI / 180) * tempObj.dist;
//                        sqrt((pow(tempObj.startDist, 2) + pow(tempObj.endDist, 2))
//                                - (2 * tempObj.startDist * tempObj.endDist * cos(tempObj.width))); // sqrt(a^2 + b^2 - 2abcos(angle)) = c

                OBJECTS[numObs] = tempObj;
                numObs++;
            }

            distCount = 0;
        }
    }

    // Second scan with PING
    for (i = 0; i < numObs; i++)
    {
        servo_to_right();
        servo_move(OBJECTS[i].angle);

        OBJECTS[i].ping = ping_read();  // Get PING at midpoint
        timer_waitMillis(100);
    }

    // Table Header
    // sprintf(DEBUG_OUTPUT, "%-12s%-12s%-12s%-12s%-12s%-12s\n\r", "Object", "Angle", "PING", "IR Distance", "Width", "Linear");

    int minWidth = OBJECTS[0].width; // Object with smallest width
    int smallIndex = 0; // Index of smallest object

    // Find min width
    for (i = 0; i < numObs; i++)
    {
        if (OBJECTS[i].width < minWidth)
        {
            minWidth = OBJECTS[i].width;
            smallIndex = i;
        }
    }

    DETECTED_OBJS = numObs;
    return OBJECTS[smallIndex];
}

/**
 *
 */
int detect_passengers() {
    detect_obj();
    NUM_PASSENGERS = DETECTED_OBJS;

    // Passenger Count to Control Center
    sprintf(DEBUG_OUTPUT, "\n\rPassenger Count: %d\n\r", NUM_PASSENGERS);
    uart_sendStr(DEBUG_OUTPUT);

    // Output Passenger List to Control Center
    sprintf(DEBUG_OUTPUT, "\n\r### List of Passengers ###\n\r%-12s%-12s%-12s%-12s%-12s\n\r", "Passenger #", "Angle", "IR Distance", "Width", "Linear");
    uart_sendStr(DEBUG_OUTPUT);

    int i;
    for (i = 0; i < NUM_PASSENGERS; i++)
    {
        sprintf(DEBUG_OUTPUT, "%-12d%-12d%-12d%-12d%-12d\n\r", i + 1, OBJECTS[i].angle, OBJECTS[i].dist, OBJECTS[i].width, OBJECTS[i].linearWidth);
        uart_sendStr(DEBUG_OUTPUT);
    }

    uart_sendStr("\n\r"); // End Table

    return NUM_PASSENGERS;
}

/**
 * Perform the CyRide 23 Orange Route test path autonomously
 * Please see the Test Field diagram for a visual path. The measurements have been
 * calculated from the diagram and used in this function.
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 */
void auto_drive(oi_t *sensor_data)
{
    move_forward_auto(sensor_data, 2030); // 203 cm
    turn_counterclockwise(sensor_data, 75); // 90 deg
    uart_sendStr("Now approaching Stop 1\n\r");
    move_forward_auto(sensor_data, 900);

    // Stop 1 reached
    if (STOP_FLAG) {
        timer_waitMillis(3000);
        STOP_FLAG = 0;
    }

    move_forward_auto(sensor_data, 365);
    turn_counterclockwise(sensor_data, 7); // 14 deg
    move_forward_auto(sensor_data, 1710);
    uart_sendStr("Now approaching Stop 2\n\r");
    turn_counterclockwise(sensor_data, 50); // 76 deg

    // Stop 2 reached
    if (STOP_FLAG) {
        timer_waitMillis(3000);
        STOP_FLAG = 0;
    }

    move_forward_auto(sensor_data, 500);
    turn_counterclockwise(sensor_data, 75); // 90 deg
    move_forward_auto(sensor_data, 1360);
    uart_sendStr("Now approaching Stop 3\n\r");
    turn_clockwise(sensor_data, 75); // 90 deg

    // Stop 3 reached
    if (STOP_FLAG) {
        timer_waitMillis(3000);
        STOP_FLAG = 0;
    }

    move_forward_auto(sensor_data, 500);
    turn_counterclockwise(sensor_data, 75); // 90 deg
    move_forward_auto(sensor_data, 1670);
    uart_sendStr("Now approaching Park & Ride Terminal\n\r");
    turn_counterclockwise(sensor_data, 75); // 90 deg
}
