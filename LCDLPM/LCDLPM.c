#include <msp430.h>

#define     _SET_OUTPUT(port, pin)  P ## port ## DIR |= BIT ## pin
#define     _SET_INPUT(port, pin)   P ## port ## DIR &= ~BIT ## pin

#define     _SET_PIN(port, pin)     P ## port ## OUT |= BIT ## pin
#define     _CLEAR_PIN(port, pin)   P ## port ## OUT &= ~BIT ## pin

#define     _FUNCTION_SET           0x3B
#define     _DISPLAY_ON             0x0D
#define     _DISPLAY_CLEAR          0x01
#define     _ENTRY_MODE             0x06
#define     _CURSOR_LEFT            0x10
#define     _CURSOR_RIGHT           0x14

/*
 * Prototypes
 */
static void Wait(volatile int time_1);
void setPorts(void);
void setInterrupts(void);
void setWriteInstruction(void);
void setWriteData(void);
void enable(void);
void setcmd(int num);
void setdata(int data);
void setLine(void);
void setCursorPOS(char direction, int amount);
void LCDinit(void);

/*
 * Globals
 */


/**
 * LCDLPM.c
 */
void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    setPorts();
    LCDinit();
    // Interrupt and LPM, comment out for polling
    setInterrupts();

    // Polling
    while(1) {
        if(P3IN & BIT6) {
            Wait(200);          // software debounce
            setLine();
        }
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

    // Polling Button
    _SET_INPUT(3, 6);

    // LCD
    _SET_OUTPUT(10, 0);         // BIT0
    _SET_OUTPUT(10, 1);         // BIT1
    _SET_OUTPUT(10, 2);         // BIT2
    _SET_OUTPUT(10, 3);         // BIT3
    _SET_OUTPUT(10, 4);         // BIT4
    _SET_OUTPUT(10, 5);         // BIT5
    _SET_OUTPUT(10, 6);         // BIT6
    _SET_OUTPUT(10, 7);         // BIT7
    _SET_OUTPUT(6, 7);          // RS
    _SET_OUTPUT(7, 4);          // RW
    _SET_OUTPUT(7, 5);          // E
}

void setInterrupts() {
    P2REN |= BIT6;                      // Enable resistor
    P2OUT |= BIT6;
    P2IE |= BIT6;                       // Enable pin interrupt
    P2IES |= BIT6;                      // Set interrupt edge
    P2IFG &= ~BIT6;                     // Clear interrupt flag

    _BIS_SR(LPM3_bits | GIE);           // Enable LPM (LPM3_bits) and global interrupts
}

void setWriteInstruction() {
    _CLEAR_PIN(6, 7);           // RS output    (0)
    _CLEAR_PIN(7, 4);           // RW output    (0)
}

void setWriteData() {
    _SET_PIN(6, 7);             // RS output    (1)
    _CLEAR_PIN(7, 4);           // RW output    (0)
}

void enable() {
    _SET_PIN(7, 5);             // E output     (1)
    _CLEAR_PIN(7, 5);           // E output     (0)
}

void setcmd(int num) {
    setWriteInstruction();
    P10OUT = num;
    enable();
    Wait(70);
}

void setdata(int data) {
    setWriteData();
    P10OUT = data;
    enable();
    Wait(70);
}

void setLine() {
    setWriteInstruction();
    int i;
    // command = 0x10
    for(i = 0; i < 40; i++) {
       setcmd(_CURSOR_RIGHT);
       Wait(20);
    }
}

void LCDinit() {
    Wait(1000);
    setcmd(_FUNCTION_SET);
    setcmd(_DISPLAY_ON);
    setcmd(_DISPLAY_CLEAR);
    setcmd(_ENTRY_MODE);
}

/*
 * ISR's
 */
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void) {
    Wait(200);      // Software debounce
    setLine();
    P2IFG &= ~BIT6;
}
