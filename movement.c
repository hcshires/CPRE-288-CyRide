/**
 * Functions to utilize various subsystems to accomplish tasks
 * Detect objects, move autonomously, etc.
 */

#include "movement.h"

/* CyBot Properties */
int NUM_PASSENGERS = 0;
int DETECTED_OBJS = 0;
char DEBUG_OUTPUT[65]; // Output message to give PuTTY

/* Global Flags */
volatile char STOP_FLAG;
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
void turn_clockwise(oi_t *sensor, double degrees)
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
void turn_counterclockwise(oi_t *sensor, double degrees)
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
 * Output the distance in centimeters the CyBot is away from an object using the onboard IR sensor
 * @param raw_val - the raw IR value from the sensor directly
 *
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
 * A specialized scan from IR to detect the number of passengers located at the starting platform
 *
 * @returns the number of passengers detected
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
 * Scan the robot path for tall objects to use in the auto IR check
 *
 * @returns the index of the objects in the way in the global objects array
 */
int scan_roadway() {
    detect_obj();

    // Only if the objects are directly in front, consider them in the way
    int i;
    for (i = 0; i < DETECTED_OBJS; i++) {
        if ((OBJECTS[i].angle <= 115 && OBJECTS[i].angle >= 75) && OBJECTS[i].dist <= 50) {
            return i;
        }
    }

    return -1;
}

/**
 * A specialized scan from IR to detect tall objects (cars, pedestrians) in the roadway and wait for them to cross
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 */
void ir_sensor_check(oi_t * sensor) {
    oi_setWheels(0, 0);
    int objIndex = scan_roadway();

    // If objects exist, alert control center until they are gone
    while (objIndex != -1) {
        uart_sendStr("ALERT! Tall object present in the roadway. Waiting for it to cross.\n\r");
        oi_setWheels(0, 0);
        objIndex = scan_roadway();
    }

    oi_setWheels(100, 100);
}

/**
 * Autonomously navigate around the object hit or detected
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int direction - The side of the CyBot that the object was detected (0 for left, 1 for right)
 *
 * @returns the x and y coordinate of the new position the robot should be for corrections
 */
Point go_around_object(oi_t *sensor, int y_remaining) {
    int tile_width = 600;//width of a hole in the ground
    int bot_width = 300;//cybot width
    int bump_width = 100;//width of a short object
    double backup_dist = 100;//distance to back up when an object is encountered
    double turn_angle = 42;//angle to turn when incrementally moving around objects
    double turn_angle_calibrated = turn_angle + 15;//adjusted value of turn_angle for use in distance calculations
    double target = backup_dist / (cos((turn_angle_calibrated * (3.14 / 180)))) + 200; //if distance traveled while avoiding object surpasses this, the bot is past the object

    Point xy_dists;//struct to store how far the bot has moved in the x and y directions while moving around an object/hole
    xy_dists.x = 0;//x distance moved
    xy_dists.y = 0;//y distance moved
    char object_type;//type of object that the bot encountered, 0 if short object, 1 if hole
    int object_width;//distance to move to get around object, value set based on object type + width of the bot

    while(OBJECT_FLAG){//main loop executes while bot is avoiding an obstacle, bot backs up, turns slightly,
                       //then attempts to continue forward to incrementally navigate around the obstacle
        move_backward(sensor, backup_dist);
        xy_dists.y -= backup_dist;//subtract backward distance from y value
        timer_waitMillis(300);
        turn_counterclockwise(sensor, turn_angle); //turn slightly to go around object
        timer_waitMillis(300);
        
        oi_setWheels(100, 100); //attempt to pass object
        double sum = 0; //distance traveled forward
        while (sum < target) { //move forward until bot is able to go around object or another object is encountered
            if(sensor->bumpLeft == 1 || sensor->bumpRight == 1){//check for bump, restarts main loop if true
                object_type = 0;
                uart_sendStr("ALERT! CyRide has hit a short object in the road.\n\r");
                break;
            } else if(sensor->cliffLeft == 1 || sensor->cliffFrontLeft == 1 || sensor->cliffFrontRight == 1 || sensor->cliffRight == 1){//check for cliff, restarts main loop if true
                object_type = 1;
                uart_sendStr("ALERT! Sinkhole in the roadway.\n\r");
                break;
            }
            sum += sensor->distance;
            xy_dists.x += sensor->distance * (sin((turn_angle_calibrated * (3.14 / 180))));//calculation for x distance traveled
            xy_dists.y += sensor->distance * (cos((turn_angle_calibrated * (3.14 / 180))));//calculation for y distance traveled
            oi_update(sensor);
        }
        stop();
        timer_waitMillis(300);
        turn_clockwise(sensor, turn_angle); //turn back to original direction to continue forward
        timer_waitMillis(300);
        if(sum >= target){//if bot is past the obstacle, move fully past the obstacle while checking for new obstacles
            if(object_type == 1){ //set distance to fully pass by the object
                object_width = tile_width + bot_width;
            } else{
                object_width = bump_width + bot_width;
            }
            ir_sensor_check(sensor);//scan for tall objects in path
            oi_setWheels(100, 100);
            sum = 0;
            while (sum < object_width) { //move forward until past object or another object is encountered
                if(sensor->bumpLeft == 1 || sensor->bumpRight == 1){//check for bump, restarts main loop if true
                    uart_sendStr("ALERT! CyRide has hit a short object in the road.\n\r");
                    object_type = 0;
                    break;
                } else if(sensor->cliffFrontLeft == 1 || sensor->cliffFrontRight == 1){ //check for cliff, restarts main loop if true
                    uart_sendStr("ALERT! Sinkhole in the roadway.\n\r");
                    object_type = 1;
                    break;
                } else if(sensor->cliffLeft == 1) {//checks for cliff on far left
                    if(object_type == 1){//indicates if bot just went around a cliff, if so turn slightly to avoid the cliff again
                        stop();
                        timer_waitMillis(300);
                        turn_clockwise(sensor, 2);
                        timer_waitMillis(300);
                        oi_setWheels(100, 100);
                    } else {//previous object was a bump, bot needs to move around this cliff, restarts main loop
                        uart_sendStr("ALERT! Sinkhole in the roadway.\n\r");
                        object_type = 1;
                        break;
                    }
                } else if(sensor->cliffRight == 1) {//checks for cliff on far right
                    if(object_type == 1){//indicates if bot just went around a cliff, if so turn slightly to avoid the cliff again
                        stop();
                        timer_waitMillis(300);
                        turn_counterclockwise(sensor, 2);
                        timer_waitMillis(300);
                        oi_setWheels(100, 100);
                    } else {//previous object was a bump, bot needs to move around this cliff, restarts main loop
                        uart_sendStr("ALERT! Sinkhole in the roadway.\n\r");
                        object_type = 1;
                        break;
                    }
                }
                if(xy_dists.y >= y_remaining) {//checks if bot has reached a point where it needs to turn, breaks out of outermost loop and returns to main program
                    stop();
                    timer_waitMillis(300);
                    OBJECT_FLAG = 0;//cleared to break outermost loop
                    sum = 0;//clears forward distance since bot needs to turn and orientation will change
                    break;
                }
                sum += sensor->distance;
                xy_dists.y += sensor->distance;
                oi_update(sensor);

            }
            stop();
            timer_waitMillis(300);
            if(sum >= object_width){//check if bot has moved fully around the encountered obstacle, returns back to path if so
                turn_clockwise(sensor, 80); //turn back towards the path
                ir_sensor_check(sensor);
                oi_setWheels(100, 100);
                sum = 0;
                double x_dist = xy_dists.x;
                while (sum < x_dist) { //move forward until back on the path or another object is encountered
                    if(sensor->bumpLeft == 1 || sensor->bumpRight == 1){//check for bump, restarts main loop if true
                        uart_sendStr("ALERT! CyRide has hit a short object in the road.\n\r");
                        object_type = 0;
                        break;
                    } else if(sensor->cliffLeft == 1 || sensor->cliffFrontLeft == 1 || sensor->cliffRight == 1 || sensor->cliffFrontRight == 1){ //check for cliff, restarts main loop if true
                        uart_sendStr("ALERT! Sinkhole in the roadway.\n\r");
                        object_type = 1;
                        break;
                    }
                    sum += sensor->distance;
                    xy_dists.x -= sensor->distance;
                    oi_update(sensor);
                }
                if(sum >= x_dist) { //made it back onto the path, break the outermost loop and return to main program
                    uart_sendStr("facing forward\n\r");
                    turn_counterclockwise(sensor, 76); //back to facing forward
                    timer_waitMillis(300);
                    OBJECT_FLAG = 0; //clear flag to end outermost loop
                }
            }
        }
    }
    return xy_dists;//returns change in x and y values
}

/**
 * Move the CyBot forward a set distance with autonomous detection and correction for objects in the path
 * Utilizes both bump and cliff sensors to avoid sudden death
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int millimeters - The distance in millimeters to move the CyBot
 *
 * @returns the distance the robot just traveled
 */
double move_forward_auto(oi_t *sensor, int millimeters) {
//    ir_sensor_check(sensor);
    oi_setWheels(100, 100); // Set power and drive baby

    int num_scans = 1;
    double sum = 0;
    Point autoPoint;
    double x_dist = 0;
    while (sum < millimeters) {

        //sprintf(DEBUG_OUTPUT, "%d\t\t%d\n\r", curAngle, measureDistIR(distancesIR[i]));
        //uart_sendStr(DEBUG_OUTPUT);

        /* IR Sensor Check */
        if (sum >= (num_scans * 500)) {
            ir_sensor_check(sensor);
            num_scans++;
        }

        /* Bump Sensor Check */
        if (sensor->bumpLeft == 1 || sensor->bumpRight == 1) { // We hit something!
            uart_sendStr("ALERT! CyRide has hit a short object in the road.\n\r");
            oi_setWheels(0, 0);
            timer_waitMillis(300);

            OBJECT_FLAG = 1;
            autoPoint = go_around_object(sensor, millimeters - sum);
            sum += autoPoint.y;
            x_dist = autoPoint.x;
            oi_setWheels(100, 100); // Set power and drive baby
        }
        
        /* Cliff Sensor Check */
        if (sensor->cliffLeft == 1 || sensor->cliffFrontLeft == 1 || sensor->cliffRight == 1 || sensor->cliffFrontRight == 1) {
            uart_sendStr("ALERT! Pot hole in the roadway.\n\r");
            oi_setWheels(0, 0);
            timer_waitMillis(300);
            
            OBJECT_FLAG = 1;
            autoPoint = go_around_object(sensor, millimeters - sum);
            sum += autoPoint.y;
            x_dist = autoPoint.x;
            oi_setWheels(100, 100); // Set power and drive baby
        }

//        /* Border Sensor Check */
//        if (sensor->lightBumpRightSignal == 1 || sensor->lightBumpCenterRightSignal == 1 || sensor->lightBumpFrontRightSignal == 1) {
//           stop();
//           timer_waitMillis(300);
//           turn_counterclockwise(sensor, 3);
//           timer_waitMillis(300);
//           oi_setWheels(100, 100);
//        }

        // lcd_printf("%lf", sum);
        sum += sensor->distance;
        oi_update(sensor);
    }

    stop();
    return x_dist;
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
    double x_dist;
    double x_dist2;
    x_dist = move_forward_auto(sensor_data, 2030); // 203 cm
    turn_counterclockwise(sensor_data, 78); // 90 deg
    timer_waitMillis(300);
    uart_sendStr("Now approaching Stop 1\n\r");
    x_dist2 = move_forward_auto(sensor_data, 900 - x_dist);

    // Stop 1 reached
    uart_sendStr("Stop 1 Reached\n\r");

    if (STOP_FLAG) {
        timer_waitMillis(3000);
        lcd_clear();
        STOP_FLAG = 0;
    }

    x_dist = move_forward_auto(sensor_data, 365 - x_dist2);
    turn_counterclockwise(sensor_data, 7); // 14 deg
    timer_waitMillis(300);
    x_dist2 = move_forward_auto(sensor_data, 1710 - x_dist);
    uart_sendStr("Now approaching Stop 2\n\r");
    turn_counterclockwise(sensor_data, 65); // 76 deg
    timer_waitMillis(300);

    // Stop 2 reached
    uart_sendStr("Stop 2 Reached\n\r");

    if (STOP_FLAG) {
        timer_waitMillis(3000);
        lcd_clear();
        STOP_FLAG = 0;
    }

    x_dist = move_forward_auto(sensor_data, 500 - x_dist2);
    turn_counterclockwise(sensor_data, 78); // 90 deg
    timer_waitMillis(300);
    x_dist2 = move_forward_auto(sensor_data, 1360 - x_dist);
    uart_sendStr("Now approaching Stop 3\n\r");
    turn_clockwise(sensor_data, 81); // 90 deg
    timer_waitMillis(300);

    // Stop 3 reached
    uart_sendStr("Stop 3 Reached\n\r");
    if (STOP_FLAG) {
        timer_waitMillis(3000);
        lcd_clear();
        STOP_FLAG = 0;
    }

    x_dist = move_forward_auto(sensor_data, 1000 - x_dist2);
    turn_counterclockwise(sensor_data, 78); // 90 deg
    timer_waitMillis(300);
    x_dist2 = move_forward_auto(sensor_data, 1400 - x_dist);
    uart_sendStr("Now approaching Park & Ride Terminal\n\r");
    turn_counterclockwise(sensor_data, 73); // 90 deg
    timer_waitMillis(300);
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
    }
}
