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
#define     _4                      0x99
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
unsigned int counter = 0;
int freq[5] = {66, 33, 22, 17, 11};

/*
 * Prototypes
 */
static void Wait(volatile int time_1);
void setPorts(void);
void setInterrupts(void);
void setSegments(int code);


/**
 * timer.c
 */
void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    setPorts();
    setInterrupts();

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
    _SET_OUTPUT(6, 7);          // Buzzer
    _SET_OUTPUT(2, 6);          // Button

    _SET_OUTPUT(10, 0);          // SEG_A   BIT0
    _SET_OUTPUT(10, 1);          // SEG_B
    _SET_OUTPUT(10, 2);          // SEG_C
    _SET_OUTPUT(10, 3);          // SEG_D
    _SET_OUTPUT(10, 4);          // SEG_E
    _SET_OUTPUT(10, 5);          // SEG_F
    _SET_OUTPUT(10, 6);          // SEG_G
    _SET_OUTPUT(10, 7);          // S_DP1

//    _SET_OUTPUT(4, 3);          // FIND USE
}

void setInterrupts() {
    //Timer0_A0 Configuration
    TA0CCTL0 |= CCIE;                   // Enable interrupt for CCR0.
    TA0CCR0 = freq[0];                  // Compare Register, f = 32768 / (TA0CCR)
    TA0CTL = TASSEL_1 + ID_0 + MC_1;    // ACLK = 32768 Hz, ACLK/1, Up Mode
    TA0CTL &= ~TAIFG;

    P2REN |= BIT6;                      // Enable resistor
    P2OUT |= BIT6;
    P2IE |= BIT6;                       // Enable pin interrupt
    P2IES |= BIT6;                      // Set interrupt edge
    P2IFG &= ~BIT6;                     // Clear interrupt flag
    _BIS_SR(GIE);                       // Enable global interrupts
}

//Timer ISR
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void)
{
    P6OUT ^= BIT7;
    TA0CTL &= ~TAIFG;
}

// Button ISR
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void) {
    Wait(200);
    counter++;
    if(counter >= 5) {
        counter = 0;
    }

    TA0CCR0 = freq[counter];
    P2IFG &= ~BIT6;
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
