/*
 * adc.c
 *
 *  Created on: Oct 20, 2022
 *      Author: hcshires
 */

#include "adc.h"
#include "Timer.h"
#include "lcd.h"

/**
 * Initialize ADC0 on the Tiva to utilize the IR sensor and SS0 sampler
 */
void adc_init(void) {
    /* Clock Config */
    // Enable clock on ADC0
    SYSCTL_RCGCADC_R |= 0b01;

    timer_waitMillis(1);

    // Enable clock on GPIO port B
    SYSCTL_RCGCGPIO_R |= 0b00000010;

    timer_waitMillis(1);

    /* GPIO Config */
    // Set PB4 as peripheral
    GPIO_PORTB_AFSEL_R |= 0b00010000;

    // Make PB4 ANALOG for ADC
    GPIO_PORTB_DEN_R &= 0b11101111;

    // Set analog to pin 4
    GPIO_PORTB_AMSEL_R |= 0b00010000;

    /* ADC Config */
    // ADC Disable
    ADC0_ACTSS_R &= 0b0000;

    // Set ADC to trigger via PSSI register
    ADC0_EMUX_R &= 0b0;

    // Configure input source samples
    ADC0_SSMUX0_R |= 0xA;

    // Configure sequence control
    ADC0_SSCTL0_R |= 0x6;

    // Configure interrupt polling
    ADC0_IM_R |= 0b0001;

    // Configure Hardware averaging PART 2
    ADC0_SAC_R |= 0x3; // 8x hardware oversampling

    // ADC Enable
    ADC0_ACTSS_R |= 0b0001;
}

/**
 * Read IR sensor values from the ADC
 */
int adc_read(void) {
    int adc_data;

    ADC0_PSSI_R |= 0b0001; // Begin sampling on SS0

    while (!(ADC0_RIS_R & 0b0001));

    adc_data = ADC0_SSFIFO0_R; // Read bit 10, AIN10
    ADC0_ISC_R |= 0b0001; // Clear interrupt

    return adc_data;
}
