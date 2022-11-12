#include <msp430.h>

/*
 * Macros
 */
#define     _SET_OUTPUT(port, pin)  P ## port ## DIR |= BIT ## pin
#define     _SET_INPUT(port, pin)   P ## port ## DIR &= ~BIT ## pin

#define     _SET_PIN(port, pin)     P ## port ## OUT |= BIT ## pin
#define     _CLEAR_PIN(port, pin)   P ## port ## OUT &= ~BIT ## pin

#define     _0                      0b11000000
#define     _1                      0b11111001
#define     _2                      0b10100100
#define     _3                      0b10110000
#define     _4                      0b10011001
#define     _5                      0b10010010
#define     _6                      0b10000010
#define     _7                      0b11111000
#define     _8                      0b10000000
#define     _9                      0b10010000
#define     _A                      0b10001000
#define     _B                      0b10000011
#define     _C                      0b11000110
#define     _D                      0b10100001
#define     _E                      0b10000110
#define     _F                      0b10001110

/*
 * Globals
 */
unsigned int counter = 0;
unsigned int counter_temp = 0;
unsigned int counter_1 = 0;
unsigned int counter_16 = 0;
int freq[5] = {66, 33, 22, 17, 11};
int characters[16] = {_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _A, _B, _C, _D, _E, _F};

int FLAG = 0;

/*
 * Prototypes
 */
static void Wait(volatile int time_1);
void setPorts(void);
void setInterrupts(void);
void clear(void);


/**
 * timer.c
 */
void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    setPorts();
    setInterrupts();
    clear();

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
    _SET_OUTPUT(2, 6);          // Interrupt Button

    _SET_OUTPUT(10, 0);         // SEG_A   BIT0
    _SET_OUTPUT(10, 1);         // SEG_B   BIT1
    _SET_OUTPUT(10, 2);         // SEG_C   BIT2
    _SET_OUTPUT(10, 3);         // SEG_D   BIT3
    _SET_OUTPUT(10, 4);         // SEG_E   BIT4
    _SET_OUTPUT(10, 5);         // SEG_F   BIT5
    _SET_OUTPUT(10, 6);         // SEG_G   BIT6
    _SET_OUTPUT(10, 7);         // S_DP1   BIT7
    _SET_OUTPUT(3, 6);          // S_DP2

    _SET_OUTPUT(4, 3);          // 7S_1       0 = TURN ON
    _SET_OUTPUT(7, 5);          // 7S_2       0 = TURN ON
}

void setInterrupts() {
    // Timer0_A0 Configuration, 1 Hz
    TA0CCTL0 |= CCIE;                   // Enable interrupt for CCR0.
    TA0CCR0 = 32768;                    // Compare Register, f = 32768 / (TA0CCR)
    TA0CTL = TASSEL_1 + ID_0 + MC_1;    // ACLK = 32768 Hz, ACLK/1, Up Mode
    TA0CTL &= ~TAIFG;

    // Timer1_A0 COnfiguration, 60 Hz
    TA1CCTL0 |= CCIE;                   // Enable interrupt for CCR1.
    TA1CCR0 = 546;                    // Compare Register, f = 32768 / (TA0CCR)
    TA1CTL = TASSEL_1 + ID_0 + MC_1;    // ACLK = 32768 Hz, ACLK/1, Up Mode
    TA1CTL &= ~TAIFG;

    P2REN |= BIT6;                      // Enable resistor
    P2OUT |= BIT6;
    P2IE |= BIT6;                       // Enable pin interrupt
    P2IES |= BIT6;                      // Set interrupt edge
    P2IFG &= ~BIT6;                     // Clear interrupt flag

    _BIS_SR(GIE);                       // Enable global interrupts
}

void clear() {
    _SET_PIN(10, 0);
    _SET_PIN(10, 1);
    _SET_PIN(10, 2);
    _SET_PIN(10, 3);
    _SET_PIN(10, 4);
    _SET_PIN(10, 5);
    _SET_PIN(10, 6);
    _SET_PIN(10, 7);
}

//Timer ISR
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void) {
    // First 7-segment display exercise
//    P10OUT = characters[counter];
//    counter++;
//    if(counter == 10) {
//        counter = 0;
//    }

    counter_1++;                // Increments every second
    if(counter_1 == 16) {
        counter_1 = 0;
    }

    counter_temp++;
    if(counter_temp == 16) {
        counter_16++;           // Increments every 16 seconds
        if(counter_16 == 16) {
            counter_16 = 0;
        }
        counter_temp = 0;
    }


    TA0CTL &= ~TAIFG;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A0_ISR(void) {
    if(FLAG == 0) {
        _SET_PIN(4, 3);                 // TURN OFF BOTH CONTROL SIGNALS
        _SET_PIN(7, 5);

        P10OUT = characters[counter_16];   // SEND DATA TO FIRST 7_SEGMENT

        _CLEAR_PIN(4, 3);               // TURN ON FIRST CONTROL SIGNAL

        FLAG = 1;
    } else {
        _SET_PIN(4, 3);                 // TURN OFF BOTH CONTROL SIGNALS
        _SET_PIN(7, 5);

        P10OUT = characters[counter_1];   // SEND DATA TO SECOND 7_SEGMENT

        _CLEAR_PIN(7, 5);               // TURN ON SECOND CONTROL SIGNAL

        FLAG = 0;
    }

    TA1CTL &= ~TAIFG;
}

// Button ISR
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void) {
    Wait(200);
    counter++;
    P2IFG &= ~BIT6;
}
