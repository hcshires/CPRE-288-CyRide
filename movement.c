#include "movement.h"
#include "helpers.h"

volatile char OVERRIDE_FLAG;
volatile char OBJECT_FLAG;

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
    while (!OVERRIDE_FLAG && abs(angle) < degrees) { // checking the angle to make sure not to surpass the user specifies
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
    while (!OVERRIDE_FLAG && abs(angle) < degrees) { // checking the angle to make sure not to surpass the user specifies
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
void go_around_object(oi_t *sensor) {
    double target = 150; //if forward distances surpasses this, bot is past the object
    
    while(OBJECT_FLAG){
        move_backward(sensor, 100); //back away from object
        timer_waitMillis(100);
        turn_counterclockwise(sensor, 15); //turn slightly to go around object
        timer_waitMillis(100);
        
        oi_setWheels(100, 100); //start going around
        double sum = 0; //distance traveled forward
        while (sum < target) { //move forward until past object or another object is encountered
            if(sensor->bumpLeft == 1 || sensor->bumpRight == 1){//check for cliffs/bump
                uart_sendStr("ALERT! CyRide has hit a short object in the road. Manual override required.\n\r");
                break;
            } else if(sensor->cliffLeft == 1 || sensor->cliffFrontLeft == 1 || sensor->cliffRight == 1 || sensor->cliffFrontRight == 1){
                uart_sendStr("ALERT! Sinkhole in the roadway. Manual override required.\n\r");
                break;
            }
            sum += sensor->distance;
            oi_update(sensor);
        }
        stop();
        timer_waitMillis(300);
        turn_clockwise(sensor, 15); //turn back to original direction
        if(sum >= target){
            OBJECT_FLAG = 0;
        }
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
    int distanceObj;
    int curAngle = 90;
    int direction = 1;

    while (!OVERRIDE_FLAG && sum < millimeters) {
        distanceObj = adc_read(); // Should be averaged value via hardware

        //sprintf(DEBUG_OUTPUT, "%d\t\t%d\n\r", curAngle, measureDistIR(distancesIR[i]));
        //uart_sendStr(DEBUG_OUTPUT);

        servo_move(5 * direction);
        if (direction == 1) {
            if (curAngle <= 180) {
                curAngle += 10;
            } else {
                direction = -1;
            }
        } else {
            if (curAngle >= 0) {
                curAngle -= 2;
            } else {
                direction = 1;
            }
        }

        /* IR Sensor Check */
        if (measureDistIR(distanceObj) < 25) {
            ir_sensor_check(sensor, sum);
        }

        /* Bump Sensor Check */
        if (sensor->bumpLeft == 1 || sensor->bumpRight == 1) { // We hit something!
            uart_sendStr("ALERT! CyRide has hit a short object in the road. Manual override required.\n\r");
            oi_setWheels(0, 0);
            timer_waitMillis(300);

            OBJECT_FLAG = 1;
            go_around_object(sensor);
            OVERRIDE_FLAG = 1;
            break;
        }
        
        /* Cliff Sensor Check */
        if (sensor->cliffLeft == 1 || sensor->cliffFrontLeft == 1 || sensor->cliffRight == 1 || sensor->cliffFrontRight == 1) {
            uart_sendStr("ALERT! Sinkhole in the roadway. Manual override required.\n\r");
            oi_setWheels(0, 0);
            timer_waitMillis(300);
            
            OBJECT_FLAG = 1;
            go_around_object(sensor);
            OVERRIDE_FLAG = 1;
            break;
        }

        // lcd_printf("%lf", sum);
        sum += sensor->distance;
        oi_update(sensor);
    }

    stop();
}

/**
 *
 */
void ir_sensor_check(oi_t * sensor, double sum) {
    oi_setWheels(0, 0);
    int objIndex = scan_roadway();

    if (objIndex != -1) {
        uart_sendStr("ALERT! Tall object present in the roadway. Manual override required.\n\r");
        oi_setWheels(0, 0);
        move_manual(sensor, &sum, OBJECTS[objIndex].dist);

        OVERRIDE_FLAG = 1;
    } else {
        oi_setWheels(100, 100);
    }
}

/**
 * Manual override to drive the CyBot
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param double *sum - Pointer to sensor's distance sum to update autonomous after manual override completes
 */
void move_manual(oi_t *sensor, double * sum, int distance) {
    while (1)
    {
        char msg = uart_receive();
        int angle = 0;

        // CyRide corrected manually, ready to return to auto drive
        if (msg == 'e') {
            break;
        }

        // If 0, Manual mode
        if (msg == 'w')
        {
            move_forward(sensor, 50);
            distance -= 5;
        }
        else if (msg == 's')
        {
            move_backward(sensor, 50);
            distance += 5;
        }
        else if (msg == 'a')
        {
            turn_counterclockwise(sensor, 5);
            angle += 5;
        }
        else if (msg == 'd')
        {
            turn_clockwise(sensor, 5);
            angle -= 5;
        }

        sprintf(DEBUG_OUTPUT, "Move %d cm, Robot at %d degrees from origin", distance, angle);
        uart_sendStr(DEBUG_OUTPUT);

    }

}
