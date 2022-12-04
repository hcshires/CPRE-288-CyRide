/*
* Custom library for implementing the UART module on the CyBot
* Used to set up the UART [UART1 at 115200]
*/

#include "uart.h"
#include "timer.h"
#include "lcd.h"

volatile char uart_data;

void uart_init(int baud)
{
    // Enable UART Module with RCGCUART
    SYSCTL_RCGCUART_R |= 0b00000010;

    timer_waitMillis(1);

    // Enable clock on GPIO port B
    SYSCTL_RCGCGPIO_R |= 0b00000010;

    timer_waitMillis(1);

    // sets PB0 and PB1 as peripherals (page 671)
    GPIO_PORTB_AFSEL_R |= 0b00000011;

    // Select U1Rx and U1Tx with PCTL pmc0 and pmc1 (page 688)  also refer to page 650
    GPIO_PORTB_PCTL_R |= 0x00000011;

    // enables pb0 and pb1
    GPIO_PORTB_DEN_R |= 0b00000011;

    // sets pb0 as output, pb1 as input
    GPIO_PORTB_DIR_R |= 0b00000001;
    GPIO_PORTB_DIR_R &= 0b11111101;

    //compute baud values [UART clock = 16 MHz]
    double fbrd;
    int    ibrd;

    // Page 896-903
    // BRD = BRDI + BRDF = UARTSysClk / (ClkDiv * Baud Rate)
    // UARTFBRD = integer(BRDF * 64 + 0.5)

    // 16 MHz clock
    //fbrd = 16000000 / (16 * baud);
    //ibrd = (int)(fbrd);
    //fbrd = (int)((fbrd - ibrd) * 64 + 0.5);
    ibrd = 8;
    fbrd = 44;

    UART1_CTL_R &= 0xFFF0; // disable UART1 (page 918)
    UART1_IBRD_R = ibrd; // write integer portion of BRD to IBRD
    UART1_FBRD_R = fbrd; // write fractional portion of BRD to FBRD
    UART1_LCRH_R = 0b01100000; // write serial communication parameters (page 916) * 8bit and no parity
    UART1_CC_R   = 0x0; // use system clock as clock source (page 939)
    UART1_CTL_R |= 0x0001; // enable UART1
}

void uart_sendChar(char data)
{
   // Only transmit when FIFO is empty
   while ((UART1_FR_R & 0x20) != 0);
   UART1_DR_R = data;
}

char uart_receive(void)
{
    char data;

    // Only receive when FIFO read is not empty
    while ((UART1_FR_R & 0x10) != 0);
    data = (char)(UART1_DR_R & 0xFF);

    return data;
}

void uart_sendStr(const char *data)
{
    while(*data != '\0')
    {
        uart_sendChar(*data);
        data++;
    }
}

void uart_interrupt_init()
{
    // Enable interrupts for receiving bytes through UART1
    UART1_IM_R |= 0b000000010000; //enable interrupt on receive - page 924

    // Find the NVIC enable register and bit responsible for UART1 in table 2-9
    // Note: NVIC register descriptions are found in chapter 3.4
    NVIC_EN0_R |= 0x00000040; //enable uart1 interrupts - page 104

    // Find the vector number of UART1 in table 2-9 ! UART1 is 22 from vector number page 104
    IntRegister(INT_UART1, uart_interrupt_handler); //give the microcontroller the address of our interrupt handler - page 104 22 is the vector number

}

void uart_interrupt_handler()
{
    // STEP 1: Check the Masked Interrupt Status
    if (UART1_MIS_R & 0b000000010000) {
        // STEP 2:  Copy the data
        uart_data = (char)(UART1_DR_R & 0xFF);

        if (uart_data == ' ') {
            oi_play_song(0); // Stop Requested Tone
            lcd_printf("STOP REQUESTED");
            uart_sendStr("Stop Requested\n\r");
            STOP_FLAG = 1;
        }

        UART1_ICR_R |= 0b00010000; // STEP 3: Interrupt Clear
    }
}
