/*
 * simpleMission.h
 *
 *  Created on: Oct 20, 2022
 *      Author: hcshires
 */

#ifndef HELPERS_H_
#define HELPERS_H_

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

/**
 * Move CyBot manually to smallest object, either autonomously or manually depending on the robot mode set by the user
 */
void simple_mission();

/**
 * Scan the environment for objects close to the Cybot using the front-facing servo IR and ultrasonic sensor
 * @returns the object with the smallest width found
 */
Obstacle detect_obj();

/**
 * Move CyBot autonomously based on the location of the smallest object passed in
 */
void auto_drive(oi_t *sensor_data);

void detect_passengers();

#endif /* HELPERS_H_ */
