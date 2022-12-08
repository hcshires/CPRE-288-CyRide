/*
 * movement.h
 *
 *  Created on: Dec 8th, 2022
 *      Author: hcshires
 */

#ifndef MOVEMENT_H_
#define MOVEMENT_H_

/* CyBot Subsystems */
#include "adc.h"
#include "button.h"
#include "lcd.h"
#include "music.h"
#include "open_interface.h"
#include "ping.h"
#include "servo.h"
#include "Timer.h"
#include "uart.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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

// Point struct to track the CyBot's location in the xy plane
typedef struct Point
{
    double x;
    double y;
} Point;

Obstacle OBJECTS[7];  // List to record found obstacles
char DEBUG_OUTPUT[65]; // Output message to give PuTTY

/**
 * Stop the CyBot (set motor power to 0)
 */
void stop();

/**
 * Move the CyBot forward a set distance
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int millimeters - The distance in millimeters to move the CyBot
 */
void move_forward(oi_t *sensor, int millimeters);

/**
 * Move the CyBot backward a set distance
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int millimeters - The distance in millimeters to move the CyBot
 */
void move_backward(oi_t *sensor, int millimeters);

/**
 * Turn the CyBot clockwise a set angle
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int degrees - The angle in degrees to move the CyBot by
 */
void turn_clockwise(oi_t *sensor, double degrees);

/**
 * Turn the CyBot counterclockwise a set angle
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int degrees - The angle in degrees to move the CyBot by
 */
void turn_counterclockwise(oi_t *sensor, double degrees);

/**
 * Output the distance in centimeters the CyBot is away from an object using the onboard IR sensor
 * @param raw_val - the raw IR value from the sensor directly
 *
 * @returns the number of cm away from an object
 */
int measureDistIR(int raw_val);

/**
 * Scan the environment for objects close to the Cybot using the front-facing servo IR and ultrasonic sensor
 * @returns the object with the smallest width found
 */
Obstacle detect_obj();

/**
 * A specialized scan from IR to detect the number of passengers located at the starting platform
 *
 * @returns the number of passengers detected
 */
int detect_passengers();

/**
 * Scan the robot path for tall objects to use in the auto IR check
 *
 * @returns the index of the objects in the way in the global objects array
 */
int scan_roadway();

/**
 * A specialized scan from IR to detect tall objects (cars, pedestrians) in the roadway and wait for them to cross
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 */
void ir_sensor_check(oi_t * sensor);

/**
 * Autonomously navigate around the object hit or detected
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int direction - The side of the CyBot that the object was detected (0 for left, 1 for right)
 *
 * @returns the x and y coordinate of the new position the robot should be for corrections
 */
Point go_around_object(oi_t *sensor, int y_remaining,);

/**
 * Move the CyBot forward a set distance with autonomous detection and correction for objects in the path
 * Utilizes both bump and cliff sensors to avoid sudden death
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int millimeters - The distance in millimeters to move the CyBot
 *
 * @returns the distance the robot just traveled
 */
double move_forward_auto(oi_t *sensor, int millimeters);

/**
 * Perform the CyRide 23 Orange Route test path autonomously
 * Please see the Test Field diagram for a visual path. The measurements have been
 * calculated from the diagram and used in this function.
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 */
void auto_drive(oi_t *sensor_data);

/**
 * Manual override to drive the CyBot
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param double *sum - Pointer to sensor's distance sum to update autonomous after manual override completes
 */
void move_manual(oi_t *sensor, double * sum, int distance);

#endif /* MOVEMENT_H_ */
