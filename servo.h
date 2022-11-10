/*
 * servo.h
 *
 *  Created on: Nov 3, 2022
 *      Author: twsmith1
 */

#ifndef SERVO_H_
#define SERVO_H_

#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"

/**
 * Initialize the servo capability using the onboard Timer in PWM mode
 */
void servo_init(void);

/*
 * Move the Servo a given number of degrees by manipulating the generated PWM waveform
 * - Use CyBot 13
   - 312000 (0xC2C0) callib (304000) = Match for 0 deg
   - 297000 (0x8828) callib (296000) = Match for 90 deg
   - 282750 (0x507E) callib (288000) = Match for 180 deg
 * @param degrees - the number of degrees to adjust the servo angle by
 *
 * @returns 1 when the servo has completed movement
 */
int servo_move(float degrees);

/**
 * Go to 180 degrees
 *
 * @returns 1 when the servo has completed movement
 */
int servo_to_left();

/**
 * Go to 0 degrees
 *
 * @returns 1 when the servo has completed movement
 */
int servo_to_right();

#endif /* SERVO_H_ */
