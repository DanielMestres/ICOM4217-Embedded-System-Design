#include <msp430.h> 


/**
 * Lab3_LCD
 */

#define     _SET_OUTPUT(port, pin)  P ## port ## DIR |= BIT ## pin

#define     _SET_PIN(port, pin)     P ## port ## OUT |= BIT ## pin
#define     _CLEAR_PIN(port, pin)   P ## port ## OUT &= ~BIT ## pin

int i;

static void Wait(volatile int time_1)
{
    volatile int time_2;
    for (time_1; time_1>1; time_1--)
    {
        for (time_2 = 110; time_2 >= 0; time_2--);
    }
}

void setPorts() {
    _SET_OUTPUT(6, 7);          // RS output
    _SET_OUTPUT(7, 4);          // RW output
    _SET_OUTPUT(7, 5);          // E output
    _SET_OUTPUT(3, 4);          // D0 output
    _SET_OUTPUT(3, 5);          // D1 output
    _SET_OUTPUT(3, 6);          // D2 output
    _SET_OUTPUT(3, 7);          // D3 output
    _SET_OUTPUT(4, 0);          // D4 output
    _SET_OUTPUT(4, 1);          // D5 output
    _SET_OUTPUT(4, 2);          // D6 output
    _SET_OUTPUT(4, 3);          // D7 output

    // Interrupt port
    // _SET_INPUT(2, 6);           // Switch interrupt input
}

void setWriteInstruction() {
    _CLEAR_PIN(6, 7);           // RS output    (0)
    _CLEAR_PIN(7, 4);           // RW output    (0)
}

void setReadInstruction() {
    _CLEAR_PIN(6, 7);           // RS output    (0)
    _SET_PIN(7, 4);             // RW output    (1)
}

void setWriteData() {
    _SET_PIN(6, 7);             // RS output    (1)
    _CLEAR_PIN(7, 4);           // RW output    (0)
}

void setReadData() {
    _SET_PIN(6, 7);             // RS output    (1)
    _SET_PIN(7, 4);             // RW output    (1)
}

void enable() {
    _SET_PIN(7, 5);             // E output     (1)
    _CLEAR_PIN(7, 5);           // E output     (0)
}

void clear() {

}

void setcmd(volatile int cmd) {
    setWriteInstruction();
    P3OUT |= ((cmd >> 0) & 1) << 4;
    P3OUT |= ((cmd >> 1) & 1) << 5;
    P3OUT |= ((cmd >> 2) & 1) << 6;
    P3OUT |= ((cmd >> 3) & 1) << 7;
    P4OUT |= ((cmd >> 4) & 1) << 0;
    P4OUT |= ((cmd >> 5) & 1) << 1;
    P4OUT |= ((cmd >> 6) & 1) << 2;
    P4OUT |= ((cmd >> 7) & 1) << 3;
    enable();

    Wait(100);
}

// SEND CHAR '', NOT STRINGS "" !!!!!!
void setdata(int data) {
    setWriteData();
    P3OUT |= ((data >> 0) & 1) << 4;
    P3OUT |= ((data >> 1) & 1) << 5;
    P3OUT |= ((data >> 2) & 1) << 6;
    P3OUT |= ((data >> 3) & 1) << 7;
    P4OUT |= ((data >> 4) & 1) << 0;
    P4OUT |= ((data >> 5) & 1) << 1;
    P4OUT |= ((data >> 6) & 1) << 2;
    P4OUT |= ((data >> 7) & 1) << 3;
    enable();

    Wait(100);
}

/*
 * Interrupt functions
 */

//void setInterrupts() {
////  P2IN = BIT6;                        // ???
//    P2REN |= BIT6;                      // Enable resistor ???
//    P2IE |= BIT6;                       // Enable pin interrupt
//    P2IES |= BIT6;                      // Set interrupt edge
//    P2IFG &= ~BIT6;                     // Clear interrupt flag
//    __bis_SR_register(GIE);             // Enable global interrupts
//}

// ISR, called automatically on switch press
//#pragma vector=PORT2_VECTOR
//__interrupt void Port_2(void) {
//    counter++;
//    P2IFG &= ~BIT6;
//}


/*
 * Legacy functions, replaced by setcmd
 */
void functionSet() {
    setWriteInstruction();

    _CLEAR_PIN(3, 4);           // D0 output    (x) (NOT NEEDED)
    _CLEAR_PIN(3, 5);           // D1 output    (x) (NOT NEEDED)
    _CLEAR_PIN(3, 6);           // D2 output    (0) (1 = 5*11, 0 = 5*8)
    _SET_PIN(3, 7);             // D3 output    (1) (1 = 2 lines, 0 = 1 line)
    _SET_PIN(4, 0);             // D4 output    (1) (1 = 8 bits, 0 = 4 bits)
    _SET_PIN(4, 1);             // D5 output    (1)
    _CLEAR_PIN(4, 2);           // D6 output    (0)
    _CLEAR_PIN(4, 3);           // D7 output    (0)

    enable();
}

void displayON() {
    setWriteInstruction();

    _SET_PIN(3, 4);             // D0 output    (1) (1 = Cursor blink ON, Cursor blink OFF)
    _CLEAR_PIN(3, 5);           // D1 output    (0) (1 = Cursor ON, 0 = Cursor OFF)
    _SET_PIN(3, 6);             // D2 output    (1) (1 = ON, 0 = OFF)
    _SET_PIN(3, 7);             // D3 output    (1)
    _CLEAR_PIN(4, 0);           // D4 output    (0)
    _CLEAR_PIN(4, 1);           // D5 output    (0)
    _CLEAR_PIN(4, 2);           // D6 output    (0)
    _CLEAR_PIN(4, 3);           // D7 output    (0)

    enable();
}

void displayClear() {
    setWriteInstruction();

    _SET_PIN(3, 4);             // D0 output    (1)
    _CLEAR_PIN(3, 5);           // D1 output    (0)
    _CLEAR_PIN(3, 6);           // D2 output    (0)
    _CLEAR_PIN(3, 7);           // D3 output    (0)
    _CLEAR_PIN(4, 0);           // D4 output    (0)
    _CLEAR_PIN(4, 1);           // D5 output    (0)
    _CLEAR_PIN(4, 2);           // D6 output    (0)
    _CLEAR_PIN(4, 3);           // D7 output    (0)

    enable();
}

void entryMode() {
    setWriteInstruction();

    _CLEAR_PIN(3, 4);           // D0 output    (0) (display shift)
    _SET_PIN(3, 5);             // D1 output    (1) (1 = increment, 0 = decrement)
    _SET_PIN(3, 6);             // D2 output    (1)
    _CLEAR_PIN(3, 7);           // D3 output    (0)
    _CLEAR_PIN(4, 0);           // D4 output    (0)
    _CLEAR_PIN(4, 1);           // D5 output    (0)
    _CLEAR_PIN(4, 2);           // D6 output    (0)
    _CLEAR_PIN(4, 3);           // D7 output    (0)

    enable();
}

void initLCD() {
    Wait(1000);
    // functionSet();
    setcmd(0x3B);
    Wait(2000);
    // functionSet();
    setcmd(0x3B);
    Wait(2000);
    // functionSet();
    // functionSet();
    setcmd(0x3B);
    setcmd(0x3B);
    displayON();
    // setcmd(0xD);          **** PROBLEM STARTS HERE
    displayClear();
    entryMode();
}

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    setPorts();
    // set interrupts
    initLCD();
}

