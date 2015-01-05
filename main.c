#include <msp430.h> 
#include <string.h>
#include <inttypes.h>

void mcu_init() {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	// Setup system @ 16Mhz
	if (CALBC1_16MHZ == 0xFF)				// If calibration constant erased
			{
		while (1)
			;                               // do not load, trap CPU!!
	}
	DCOCTL = 0;                          // Select lowest DCOx and MODx settings
	BCSCTL1 = CALBC1_16MHZ;                   // Set range
	DCOCTL = CALDCO_16MHZ;                    // Set DCO step + modulation*/
}

void uart_setup() {
	// setup uart @ 9600 baud
	P1SEL |= BIT1 | BIT2;
	P1SEL2 |= BIT1 | BIT2;
	UCA0CTL1 |= UCSSEL_2;
	UCA0BR0 = 130;
	UCA0BR1 = 6;
	UCA0MCTL = UCBRS1 + UCBRS2;
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

int main(void) {
	mcu_init();
	uart_setup();
	while (1) {
		serial_println("Yo YO rocky!!!");
	}
}
