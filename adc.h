/*
 * adc.h
 *
 *  Created on: Oct 20, 2022
 *      Author: hcshires
 */

#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"

/**
 * Initialize ADC0 on the Tiva to utilize the IR sensor and SS0 sampler
 */
void adc_init(void);

/**
 * Read IR sensor values from the ADC
 */
int adc_read(void);

#endif /* ADC_H_ */
