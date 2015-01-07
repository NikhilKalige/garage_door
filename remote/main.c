#include <msp430.h> 
#include <string.h>
#include <inttypes.h>
#include "ti/include.h"

#define MAG_IN	BIT4
#define DEBUG

// time defs @ VLO = 12K/4 = 3KHz
#define msec_500		1500
#define sec1				3000
#define sec5				15000

void mcu_init();
void uart_setup();
void serial_print_char(unsigned char val) ;
void serial_print(unsigned char* data) ;
void serial_println(unsigned char* data) ;
void timera_setup();
void timera_start(int time);
void port_setup();

volatile uint8_t flag;
volatile uint8_t timer_flag;
volatile uint8_t mag_flag;
uint8_t val;
char tx_packet[3] = {2, 0x01, 0x00};
char rxBuffer[12];

int main(void) {
	mcu_init();
	port_setup();
	uart_setup();
#ifdef DEBUG
	serial_println("Starting !!");
#endif
	// 5ms delay to compensate for time to startup between MSP430 and CC1100/2500
	__delay_cycles(5000);
	RF_init();

#ifdef DEBUG
	serial_println("RF Setup done !!");
#endif
    timera_setup();
	__enable_interrupt();
	while (1) {
		Radio_GotoSleep();
		timera_start(msec_500);
		__bis_SR_register(LPM3_bits);
		Radio_WakeUp();
		// turn on and poll the radio for 500ms
		timera_start(msec_500);
		__bis_SR_register(LPM3_bits);
		if(mag_flag == 1)
			serial_println("Door open");
		else
			serial_println("Door closed");
	}
}

void mcu_init() {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	// Setup system @ 16Mhz
	/*
	if (CALBC1_16MHZ == 0xFF)	{			// If calibration constant erased
		while (1);                               // do not load, trap CPU!!
	}
	DCOCTL = 0;                          // Select lowest DCOx and MODx settings
	BCSCTL1 = CALBC1_16MHZ | DIVA_2;                   // Set range and ACLK at VLO/4
	BCSCTL3 |= LFXT1S_2;
	DCOCTL = CALDCO_16MHZ;                    // Set DCO step + modulation
 	*/

	// Setup @1MHz
	if (CALBC1_1MHZ==0xFF) {				// If calibration constant erased
	    while(1);                               // do not load, trap CPU!!
	}
	DCOCTL = 0;                               // Select lowest DCOx and MODx settings
	BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
	DCOCTL = CALDCO_1MHZ;

	BCSCTL1 |=  DIVA_2;                   // Set range and ACLK at VLO/4
	BCSCTL3 |= LFXT1S_2;
}

// Setup timerA as compare with ACLK
void timera_setup() {
	TA0CTL |= TASSEL_1;
}

void timera_start(int time) {
	TA0CCR0 = time;
	TA0CTL |= (MC_1 | TAIE);
	TA0CTL &= ~TAIFG;
}

void timera_disable() {
	TA0CTL &= ~(MC_1 | TAIE);
}

void port_setup() {
	P1DIR &= ~MAG_IN;
	// Setup pullup resistor
	P1OUT |= MAG_IN;
	P1REN |= MAG_IN;
	// Enable HtoL interrupt
	P1IES |= MAG_IN;
	P1IFG = 0;
	P1IE |= MAG_IN;
}

/* Define interrupt vector */
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
	flag = 1;
	P1IFG &= ~MAG_IN;
	P1IE &= ~MAG_IN;
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void) {
	if(TI_CC_GDO0_PxIFG & TI_CC_GDO0_PIN) {
		char status[2];
		char len = 11;
		if(RFReceivePacket(rxBuffer,&len,status)) {
			if(rxBuffer[1] == 0xEE)
				mag_flag = 1;
			else
				mag_flag = 0;
		}
	}
	TI_CC_GDO0_PxIFG &= ~TI_CC_GDO0_PIN;      // After pkt RX, this flag is set.
	__bic_SR_register_on_exit(LPM3_bits);
}


#pragma vector=TIMER0_A1_VECTOR
__interrupt void timer_interrupt(void) {
	uint8_t int_val;
	int_val = TA0IV;
	if(int_val & TA1IV_TAIFG) {
		timer_flag = 1;
		TA0CCTL0 &= ~CCIFG;
		timera_disable();
		__bic_SR_register_on_exit(LPM3_bits);
	}
}

void uart_setup() {
	P1SEL |= BIT1 | BIT2;
	P1SEL2 |= BIT1 | BIT2;
	UCA0CTL1 |= UCSSEL_2;
	// setup uart 9600 baud @16MHz
	/*
	UCA0BR0 = 130;
	UCA0BR1 = 6;
	UCA0MCTL = UCBRS1 + UCBRS2;
	*/
	// setup uart 9600 baud @1MHz
    UCA0BR0 = 104;
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS0;

	UCA0CTL1 &= ~UCSWRST;
	//IE2 |= UCA0RXIE ;
}

void serial_print_char(unsigned char val) {
	while (UCA0STAT & UCBUSY);
	UCA0TXBUF = val;
}

void serial_print(unsigned char* data) {
	uint8_t i, len;
	len = strlen(data);
	for (i = 0; i < len; i++) {
		serial_print_char(data[i]);
	}
}

void serial_println(unsigned char* data) {
	serial_print(data);
	serial_print_char('\n');
}

