#include "open_interface.h"
#include "uart.h"
#include <stdio.h>
#include <stdlib.h>

volatile char flag; // Your UART interrupt can update this flag

void stop() {
    flag = 0;
    oi_setWheels(0, 0);
}

void move_forward(oi_t *sensor, int millimeters)
{
    oi_setWheels(100, 100); // move full speed ahead

    double sum = 0;
    while (sum < millimeters && !flag) {
        sum += sensor->distance;
        oi_update(sensor);
    }

    stop();
}

void move_backward(oi_t *sensor, int millimeters)
{
    oi_setWheels(-100, -100); // move full speed back

    double sum = 0;
    while (sum < millimeters && !flag) {
        sum -= sensor->distance;
        oi_update(sensor);
    }

    stop();
}

void turn_clockwise(oi_t *sensor, int degrees)
{
    oi_setWheels(-100, 100); // turn via one wheel only

    double angle = 0;
    while (abs(angle)  < degrees && !flag) { // checking the angle to make sure not to surpass the user specifies
      angle += sensor->angle;
      oi_update(sensor);
    }

    stop();
}

void turn_counterclockwise(oi_t *sensor, int degrees)
{
    oi_setWheels(100, -100); // turn via one wheel only

    double angle = 0;
    while (abs(angle) < degrees && !flag) { // checking the angle to make sure not to surpass the user specifies
       angle += sensor->angle;
       oi_update(sensor);
    }

    stop();
}

void avoid_bump(oi_t *sensor, int direction)
{
    move_backward(sensor, 250);

    if (direction == 0){ // If left bumper or both are hit
        turn_clockwise(sensor, 75);
    }

    else { // Otherwise right bumper
        turn_counterclockwise(sensor, 75);
    }

    move_forward(sensor, 100);

    if (direction == 0){
       turn_counterclockwise(sensor, 75);
    }

    else{
       turn_clockwise(sensor, 75);
    }
}

void avoid_bump_angle(oi_t *sensor, int angle) {
    move_backward(sensor, 250);

    if (angle <= 90) {
        turn_clockwise(sensor, 75);
    } else {
        turn_counterclockwise(sensor, 75);
    }

    move_forward(sensor, 100);

    if (angle <= 90){
       turn_counterclockwise(sensor, 75);
    }

    else{
       turn_clockwise(sensor, 75);
    }
}

void move_forward_bump(oi_t *sensor, int millimeters) {
    oi_setWheels(100, 100); // Set power and drive baby

    double sum = 0;
    while (sum < millimeters && !flag) {
        sum += sensor->distance;
        oi_update(sensor);

        if (sensor->bumpLeft == 1) { // We hit something!
            oi_setWheels(0, 0);
            timer_waitMillis(300);
            avoid_bump(sensor, 0); // Defaults to left if both are 1 since left is checked first
            break;
        } else if (sensor->bumpRight == 1) {
            oi_setWheels(0, 0);
            timer_waitMillis(300);
            avoid_bump(sensor, 1);
            break;
        }
    }

    stop();
}

int move_forward_bump_auto(oi_t *sensor, int millimeters, int angle) {
    oi_setWheels(100, 100); // Set power and drive baby
    int bumped = 0;

    double sum = 0;
    while (sum < millimeters && !flag) {
        sum += sensor->distance;
        oi_update(sensor);

        if (sensor->bumpLeft == 1 || sensor->bumpRight == 1) { // We hit something!
            oi_setWheels(0, 0);
            timer_waitMillis(300);
            avoid_bump_angle(sensor, angle); // Defaults to left if both are 1 since left is checked first
            bumped = 1;
            break;
        }
    }

    stop();
    return bumped;
}
