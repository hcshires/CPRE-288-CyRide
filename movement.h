#include "open_interface.h"

void stop(oi_t *sensor);

void move_forward(oi_t *sensor, int centimeters);
void move_forward_bump(oi_t *sensor, int centimeters);
int move_forward_bump_auto(oi_t *sensor, int centimeters, int angle);
void move_backward(oi_t *sensor, int centimeters);

void turn_counterclockwise(oi_t *sensor, int degrees);
void turn_clockwise(oi_t *sensor, int degrees);

void avoid_bump(oi_t *sensor, int direction);
void avoid_cliff(oi_t *sensor);
