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

void adc_init(void);
int adc_read(void);

#endif /* ADC_H_ */
