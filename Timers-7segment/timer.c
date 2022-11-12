#include <msp430.h> 

/*
 * Macros
 */
#define     _SET_OUTPUT(port, pin)  P ## port ## DIR |= BIT ## pin
#define     _SET_INPUT(port, pin)   P ## port ## DIR &= ~BIT ## pin

#define     _SET_PIN(port, pin)     P ## port ## OUT |= BIT ## pin
#define     _CLEAR_PIN(port, pin)   P ## port ## OUT &= ~BIT ## pin

#define     _0                      0xC0
#define     _1                      0xF9
#define     _2                      0xA4
#define     _3                      0xB0
#define     _4                      0x93
#define     _5                      0x92
#define     _6                      0x82
#define     _7                      0xF8
#define     _8                      0x80
#define     _9                      0x90
#define     _A                      0x88
#define     _B                      0x83
#define     _C                      0x86
#define     _D                      0xA1
#define     _E                      0x86
#define     _F                      0x8E



/*
 * Globals
 */
unsigned int count;

/*
 * Prototypes
 */
static void Wait(volatile int time_1);
void setPorts(void);
void setTimer(void);
void setSegments(int code);


/**
 * timer.c
 */
void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	setPorts();
//	setTimer();
//	setSegments(_A);

	while(1) {
	    P7OUT ^= BIT5;
	    Wait(10);
	    P3OUT ^= BIT4;
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
    _SET_OUTPUT(6, 7);          // Buzzer
    _SET_OUTPUT(7, 4);          // Button

    _SET_OUTPUT(7, 5);          // SEG_A
    _SET_OUTPUT(3, 4);          // SEG_B
    _SET_OUTPUT(3, 5);          // SEG_C
    _SET_OUTPUT(3, 6);          // SEG_D
    _SET_OUTPUT(3, 7);          // SEG_E
    _SET_OUTPUT(4, 0);          // SEG_F
    _SET_OUTPUT(4, 1);          // SEG_G
    _SET_OUTPUT(4, 2);          // S_DP1

//    _SET_OUTPUT(4, 3);          // FIND USE
}

void setTimer() {
    //Timer0_A0 Configuration
    TA0CCTL0 |= CCIE;                   // Enable interrupt for CCR0.
    TA0CCR0 = 16;                       // Compare Register, f = 32768 / (2*TA0CCR), f(16) = 1024 Hz
    TA0CTL = TASSEL_1 + ID_0 + MC_1;    // ACLK = 32768 Hz, ACLK/1 ???, Up Mode
    _BIS_SR(GIE);                       // Enable global interrupts
}

//Timer ISR
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void)
{
    count++;
    P6OUT ^= BIT7;
    TA0CTL &= ~TAIFG;
}

void setSegments(int code) {
    P7OUT |= ((code >> 0) & 1) << 5;
    P3OUT |= ((code >> 1) & 1) << 4;
    P3OUT |= ((code >> 2) & 1) << 5;
    P3OUT |= ((code >> 3) & 1) << 6;
    P3OUT |= ((code >> 4) & 1) << 7;
    P4OUT |= ((code >> 5) & 1) << 0;
    P4OUT |= ((code >> 6) & 1) << 1;
    P4OUT |= ((code >> 7) & 1) << 2;
}
