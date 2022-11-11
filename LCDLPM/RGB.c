#include <msp430.h> 

#define     _SET_OUTPUT(port, pin)  P ## port ## DIR |= BIT ## pin
#define     _SET_INPUT(port, pin)   P ## port ## DIR &= ~BIT ## pin

#define     _SET_PIN(port, pin)     P ## port ## OUT |= BIT ## pin
#define     _CLEAR_PIN(port, pin)   P ## port ## OUT &= ~BIT ## pin

/*
 * Prototypes
 */
static void Wait(volatile int time_1);
void setPorts(void);
void setPWM(void);

/*
 * Globals
 */
int row = 0;
int colors[8][3] = {
                    {0,0,500},
                    {0,500,0},
                    {500,0,0},
                    {500,61,424},
                    {61,440,500},
                    {470,394,76},
                    {500,242,61},
                    {500,500,500}
};

/**
 * RGB.c
 */
void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	setPorts();
	setPWM();

	while(1) {

	}
}

/*
 * Implementation
 */
static void Wait(volatile int time_1) {
    volatile int time_2;
    for (time_1; time_1>1; time_1--)
    {
        for (time_2 = 110; time_2 >= 0; time_2--);
    }
}

void setPorts() {
    // Interrupt Button
    _SET_INPUT(2, 6);

    // PWM's
    _SET_OUTPUT(1, 4);          // TA0 PWM  // RED
    _SET_OUTPUT(4, 2);          // TB0 PWM  // GREEN
    _SET_OUTPUT(7, 3);          // TA1 PWM  // BLUE

    _SET_OUTPUT(4, 7);          // TOGGLE
}

void setPWM() {
    // PWM's, at 1000 Hz
    // Timer0_A0 Configuration (TA0)
    P1SEL |= BIT4;
    TA0CCR0 = 500;                     // Compare Register
    TA0CCTL3 = OUTMOD_7;                // Reset output mode
    TA0CCR3 = 0;
    TA0CTL |= (TASSEL_2 | ID_1 | MC_1);  // SMCLK 1 MHz, Up Mode, /2 prescaler

    TA0CCTL0 |= CCIE;                   // Enable interrupt for CCR0.
    TA0CTL &= ~TAIFG;

    // Timer0_B0 Configuration (TB0)
    P4SEL |= BIT2;
    TB0CCR0 = 500;                     // Compare Register
    TB0CCTL2 = OUTMOD_7;                // Reset output mode
    TB0CCR2 = 0;
    TB0CTL |= (TASSEL_2 | ID_1 | MC_1);           // ACLK, Up Mode /2 prescaler

    // Timer0_A1 Configuration (TA1)
    P7SEL |= BIT3;
    TA1CCR0 = 500;                     // Compare Register
    TA1CCTL2 = OUTMOD_7;                // Reset output mode
    TA1CCR2 = 500;
    TA1CTL |= (TASSEL_2 | ID_1 | MC_1);           // ACLK, Up Mode /2 prescaler

    P2REN |= BIT6;                      // Enable resistor
    P2OUT |= BIT6;
    P2IE |= BIT6;                       // Enable pin interrupt
    P2IES |= BIT6;                      // Set interrupt edge
    P2IFG &= ~BIT6;                     // Clear interrupt flag

    _BIS_SR(LPM3_bits | GIE);           // Enable global interrupts, low power mode
}

/*
 * ISR's
 */
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void)
{


    TA0CTL &= ~TAIFG;
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void) {
    Wait(200);
    row++;
    if(row == 8) {
        row = 0;
    }

    TA0CCR3 = colors[row][0];
    TB0CCR2 = colors[row][1];
    TA1CCR2 = colors[row][2];
    P2IFG &= ~BIT6;
}
