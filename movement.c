#include "movement.h"
#include "helpers.h"

/**
 * Stop the CyBot (set motor power to 0)
 */
void stop()
{
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
    while (sum < millimeters) {
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
    while (sum < millimeters) {
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
    while (abs(angle) < degrees) { // checking the angle to make sure not to surpass the user specifies
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
    while (abs(angle) < degrees) { // checking the angle to make sure not to surpass the user specifies
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
void move_forward_auto(oi_t *sensor, int millimeters) {
    oi_setWheels(100, 100); // Set power and drive baby

    double sum = 0;
    while (sum < millimeters) {
        /* IR Sensor Check every 50 cm */
        if ((int)sum % 50 == 0) {
            oi_setWheels(0, 0);

            if (scan_roadway()) {
                uart_sendStr("ALERT! Tall object present in the roadway. Manual override required.\n\r");
                oi_setWheels(0, 0);
                move_manual(sensor, &sum);
            } else {
                oi_setWheels(100, 100);
            }
        }

        /* Bump Sensor Check */
        if (sensor->bumpLeft == 1 || sensor->bumpRight == 1) { // We hit something!
            uart_sendStr("ALERT! CyRide has hit a short object in the road. Manual override required.\n\r");
            oi_setWheels(0, 0);
            timer_waitMillis(300);

            move_backward(sensor, 100);
            sum -= 100;
            move_manual(sensor, &sum);
//          go_around_object(sensor, 0); // Defaults to left if both are 1 since left is checked first
//          sum += 636.4; //adds forward distance from going around an object to sum

//          go_around_object(sensor, 1); // Defaults to left if both are 1 since left is checked first
//          sum += 636.4; //adds forward distance from going around an object to sum

            oi_setWheels(100, 100); // Set power and drive baby
        }
        
        /* Cliff Sensor Check */
        if (sensor->cliffLeft == 1 || sensor->cliffFrontLeft == 1 || sensor->cliffRight == 1 || sensor->cliffFrontRight == 1) {
            uart_sendStr("ALERT! Sinkhole in the roadway. Manual override required.\n\r");
            oi_setWheels(0, 0);
            timer_waitMillis(300);
            move_backward(sensor, 100);
            sum -= 100;
            move_manual(sensor, &sum);

            oi_setWheels(100, 100);
        }

        sum += sensor->distance;
        oi_update(sensor);
    }

    stop();
}

/**
 * Manual override to drive the CyBot
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param double *sum - Pointer to sensor's distance sum to update autonomous after manual override completes
 */
void move_manual(oi_t *sensor, double * sum) {
    while (1)
    {
        char msg = uart_receive();

        // CyRide corrected manually, ready to return to auto drive
        if (msg == 'e') {
            break;
        }

        // If 0, Manual mode
        if (msg == 'w')
        {
            oi_setWheels(100, 100);
            *sum += sensor->distance;
        }
        else if (msg == 's')
        {
            oi_setWheels(-100, -100);
            *sum -= sensor->distance;
        }
        else if (msg == 'a')
        {
            oi_setWheels(100, -100);
        }
        else if (msg == 'd')
        {
            oi_setWheels(-100, 100);
        }

        oi_update(sensor);

        timer_waitMillis(500);
        oi_setWheels(0, 0);

    }

}
