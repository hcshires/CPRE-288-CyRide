#include "open_interface.h"
#include "uart.h"

#include <stdio.h>
#include <stdlib.h>

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
void turn_clockwise(oi_t *sensor, int degrees);

/**
 * Turn the CyBot counterclockwise a set angle
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int degrees - The angle in degrees to move the CyBot by
 */
void turn_counterclockwise(oi_t *sensor, int degrees);

/**
 * Autonomously navigate around the object hit or detected
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int direction - The side of the CyBot that the object was detected (0 for left, 1 for right)
 */
void go_around_object(oi_t *sensor, int direction);

/**
 * Move the CyBot forward a set distance with autonomous detection and correction for objects in the path
 * Utilizes both bump and cliff sensors to avoid sudden death
 *
 * @param oi_t *sensor - Sensor object to store flags and status
 * @param int millimeters - The distance in millimeters to move the CyBot
 */
void move_forward_auto(oi_t *sensor, int millimeters);

/**
 *
 */
void move_manual(oi_t *sensor, double * sum);
