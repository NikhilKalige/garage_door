#include <msp430.h> 

void mcu_init() {
	 WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	 // Setup system @ 16Mhz
	if (CALBC1_16MHZ==0xFF)					// If calibration constant erased
	{
		while(1);                               // do not load, trap CPU!!
	}
	DCOCTL = 0;                               // Select lowest DCOx and MODx settings
	BCSCTL1 = CALBC1_16MHZ;                   // Set range
	DCOCTL = CALDCO_16MHZ;                    // Set DCO step + modulation*/
}

void uart_setup() {
	// setup uart @ 9600 baud
	P1SEL |= BIT1 | BIT2 ;
	P1SEL2 |= BIT1 | BIT2 ;
	UCA0CTL1 |= UCSSEL_2 ;
	UCA0BR0 = 130 ;
	UCA0BR1 = 6 ;
	UCA0MCTL = UCBRS1 + UCBRS2;
	UCA0CTL1 &= ~UCSWRST ;
	//IE2 |= UCA0RXIE ;
}

void uart_send_char(unsigned char val) {
	while(UCA0STAT & UCBUSY);
	UCA0TXBUF = val ;
}

void uart_send_data(unsigned char * data, unsigned char length) {
	unsigned int i ;
	for(i = 0 ; i < length ; i ++) {
		uart_send_char(data[i]);
	}
}



int main(void) {
	mcu_init();
	uart_setup();
	while(1) {
		uart_send_data("Yo YO rocky!!!", 14);
	}
}
