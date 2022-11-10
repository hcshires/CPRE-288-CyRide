/*
 * servo.c
 *
 *  Created on: Nov 3, 2022
 *      Author: twsmith1
 */

#include "servo.h"
#include "Timer.h"
#include "lcd.h"

#define ZERO_DEG 311910
#define MAX_DEG 284856

int steps = (ZERO_DEG - MAX_DEG) / 180.0;

// Use CyBot 6
// 311910 (0xC266) callib (304000) = Match for 0 deg
// 298500 (0x8E04) callib (296000) = Match for 90 deg
// 284856 (0x58B8) callib (288000) = Match for 180 deg

/**
 * Initialize the servo capability using the onboard Timer in PWM mode
 */
void servo_init(void) {
    // Enable clock on GPIO port B
    SYSCTL_RCGCGPIO_R |= 0b00000010;

    timer_waitMillis(1);

    // Enable Timer 1
    SYSCTL_RCGCTIMER_R |= 0b00000010;

    timer_waitMillis(1);

    // Enable PB5
    GPIO_PORTB_DEN_R |= 0b00100000;

    // Select Timer Function
    GPIO_PORTB_DIR_R |= 0b00100000;
    GPIO_PORTB_AFSEL_R |= 0b00100000;
    GPIO_PORTB_PCTL_R |= 0x00700000;

    /* Configure GPTM Input Edge Time Mode - Using Timer 1B */
    TIMER1_CTL_R &= 0b1111111011111110; // Disable before changes
    TIMER1_CFG_R = 0x4; // Split mode
    TIMER1_TBMR_R = 0b1010; // Count down, capture mode with PWM

    /* 24-bit Timer: 16-bit ILR + 8-bit Pre-scaler */
    TIMER1_TBILR_R = 0xE200; // Servo Period 20 ms, 320,000 clock ticks per period
    TIMER1_TBPR_R = 0x04; // Preset value
    TIMER1_TBMATCHR_R = 0x8E04; // Match value init at 90 deg
    TIMER1_TBPMR_R = 0x04; // Prest init (same for 0-180 deg)

    TIMER1_CTL_R |= 0b0000000100000000;
}

/*
 * Move the Servo a given number of degrees by manipulating the generated PWM waveform
 * @param degrees - the number of degrees to adjust the servo angle by
 *
 * @returns the current servo position count
 */
int servo_move(float degrees) {
    // Don't move past 180 or 0
    if (TIMER1_TBMATCHR_R >= 0xC2C1) {
        TIMER1_TBMATCHR_R = 0xC2C0; // Reached 0 deg
    } else if (TIMER1_TBMATCHR_R <= 0x58B7) {
        TIMER1_TBMATCHR_R = 0x58B8; // Reached 180 deg
    } else {
        TIMER1_TBMATCHR_R -= (steps * degrees);
    }

    timer_waitMillis(150);
    return TIMER1_TBMATCHR_R + 0x040000;
}

/**
 * Go to 180 degrees
 *
 * @returns the current servo position count
 */
int servo_to_left() {
    TIMER1_TBMATCHR_R = 0x507E;

    timer_waitMillis(15);
    return TIMER1_TBMATCHR_R + 0x040000;
}

/**
 * Go to 0 degrees
 *
 * @returns the current servo position count
 */
int servo_to_right() {
    TIMER1_TBMATCHR_R = 0xC2C0;

    timer_waitMillis(15);
    return TIMER1_TBMATCHR_R + 0x040000;
}
