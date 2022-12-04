#include "open_interface.h"

volatile char FLAG; // Your UART interrupt can update this flag

/**
 * Stop the CyBot (set motor power to 0)
 */
void stop()
{
    FLAG = 0;
    oi_setWheels(0, 0);
}

/**
 * Move the CyBot forward a set distance
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int millimeters - The distance in millimeters to move the CyBot
 */
void move_forward(oi_t *sensor, int millimeters)
{
    oi_setWheels(100, 100); // move full speed ahead

    double sum = 0;
    while (sum < millimeters && !FLAG) {
        sum += sensor->distance;
        oi_update(sensor);
    }

    stop();
}

/**
 * Move the CyBot backward a set distance
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int millimeters - The distance in millimeters to move the CyBot
 */
void move_backward(oi_t *sensor, int millimeters)
{
    oi_setWheels(-100, -100); // move full speed back

    double sum = 0;
    while (sum < millimeters && !FLAG) {
        sum -= sensor->distance;
        oi_update(sensor);
    }

    stop();
}

/**
 * Turn the CyBot clockwise a set angle
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int degrees - The angle in degrees to move the CyBot by
 */
void turn_clockwise(oi_t *sensor, int degrees)
{
    oi_setWheels(-100, 100); // turn via one wheel only

    double angle = 0;
    while (abs(angle)  < degrees && !FLAG) { // checking the angle to make sure not to surpass the user specifies
      angle += sensor->angle;
      oi_update(sensor);
    }

    stop();
}

/**
 * Turn the CyBot counterclockwise a set angle
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int degrees - The angle in degrees to move the CyBot by
 */
void turn_counterclockwise(oi_t *sensor, int degrees)
{
    oi_setWheels(100, -100); // turn via one wheel only

    double angle = 0;
    while (abs(angle) < degrees && !FLAG) { // checking the angle to make sure not to surpass the user specifies
       angle += sensor->angle;
       oi_update(sensor);
    }

    stop();
}

/**
 * Autonomously navigate around the object hit or detected
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int direction - The side of the CyBot that the object was detected (0 for left, 1 for right)
 */
void go_around_object(oi_t *sensor, int direction) {
    move_backward(sensor, 250);

    if (direction == 0) { // Left or both sides
        turn_clockwise(sensor, 45);
        move_forward(sensor, 450);
        turn_counterclockwise(sensor, 90);
        move_forward(sensor, 450);
        turn_clockwise(sensor, 45);
    } else { // Ride side
        turn_counterclockwise(sensor, 45);
        move_forward(sensor, 450);
        turn_clockwise(sensor, 90);
        move_forward(sensor, 450);
        turn_counterclockwise(sensor, 45);
    }
}

/**
 * Move the CyBot forward a set distance with autonomous detection and correction for objects in the path
 * Utilizes both bump and cliff sensors to avoid sudden death
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int millimeters - The distance in millimeters to move the CyBot
 */
void move_forward_auto(oi_t *sensor, int millimeters) { //in progress
    oi_setWheels(100, 100); // Set power and drive baby

    double sum = 0;
    while (sum < millimeters && !FLAG) {
        sum += sensor->distance;
        oi_update(sensor);

        /* Bump Sensor Check */
        if (sensor->bumpLeft == 1) { // We hit something on the left!
            uart_sendStr("ALERT! Object present in the roadway. Stopping and waiting for crew...\n\r");
            oi_setWheels(0, 0);
            timer_waitMillis(300);
            go_around_object(sensor, 0); // Defaults to left if both are 1 since left is checked first
            sum += 636.4; //adds forward distance from going around an object to sum
        } else if (sensor->bumpRight == 1) { // We hit something on the left!
            uart_sendStr("ALERT! Object present in the roadway. Stopping and waiting for crew...\n\r");
            oi_setWheels(0, 0);
            timer_waitMillis(300);
            go_around_object(sensor, 1); // Defaults to left if both are 1 since left is checked first
            sum += 636.4; //adds forward distance from going around an object to sum
        }
        
        /* Cliff Sensor Check */
        if (sensor->cliffLeft == 1 || sensor->cliffFrontLeft == 1) {
            uart_sendStr("ALERT! Sinkhole in the roadway. Please dispatch repair crew to CyRide Orange Route and correct the problem.\n\r");
            oi_setWheels(0, 0);
            timer_waitMillis(300);
            move_backward(sensor, 100);
            sum -= 100;
            turn_clockwise(sensor, 90);
            move_forward(sensor, 100);
            turn_counterclockwise(sensor, 90);
        } else if (sensor->cliffRight == 1 || sensor->cliffFrontRight == 1) {
            uart_sendStr("ALERT! Sinkhole in the roadway. Please dispatch repair crew to CyRide Orange Route and correct the problem.\n\r");
            oi_setWheels(0, 0);
            timer_waitMillis(300);
            move_backward(sensor, 100);
            sum -= 100;
            turn_counterclockwise(sensor, 90);
            move_forward(sensor, 100);
            turn_clockwise(sensor, 90);
        }
    }

    stop();
}
