#include <msp430.h>

#define     _SET_OUTPUT(port, pin)  P ## port ## DIR |= BIT ## pin
#define     _SET_INPUT(port, pin)   P ## port ## DIR &= ~BIT ## pin

#define     _SET_PIN(port, pin)     P ## port ## OUT |= BIT ## pin
#define     _CLEAR_PIN(port, pin)   P ## port ## OUT &= ~BIT ## pin

#define     _FUNCTION_SET           0x3B
#define     _DISPLAY_ON             0x0D
#define     _DISPLAY_CLEAR          0x01
#define     _ENTRY_MODE             0x06

// Global counter
volatile int counter = 0;
char countStr[] = "000";

// Actualiza el countStr, FIX ????
void updateCountStr() {
    if(counter==0) return;

    int temp = counter;       // <- como counter es global, pues pa no cambiarlo
    int i = 0;                  // ???
    for(temp > 0; temp /= 10; i++){

        //si no hacemos esto, el string se guarda al reves. es decir
        //si count es 123, el countStr[] lo guardaria como 321
        countStr[strlen(countStr)-1-i]= temp%10 + '0';
    }

}

// Global flag
volatile int PUSH_FLAG = 0;

static void Wait(volatile int time_1) {
    volatile int time_2;
    for (time_1; time_1>1; time_1--)
    {
        for (time_2 = 110; time_2 >= 0; time_2--);
    }
}

void setLCDPorts() {
    // LCD interface ports
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
}

void setKeypadPorts() {
    _SET_OUTPUT(10, 3);         // Row 1
    _SET_OUTPUT(10, 4);         // Row 2
    _SET_OUTPUT(10, 5);         // Row 3
    _SET_OUTPUT(10, 6);         // Row 4

    _SET_INPUT(1, 2);          // Col 1
    _SET_INPUT(1, 5);          // Col 2
    _SET_INPUT(1, 6);          // Col 3
}

/*
 * LCD functions
 */

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

void clearpins() {
    _CLEAR_PIN(3, 4);
    _CLEAR_PIN(3, 5);
    _CLEAR_PIN(3, 6);
    _CLEAR_PIN(3, 7);
    _CLEAR_PIN(4, 0);
    _CLEAR_PIN(4, 1);
    _CLEAR_PIN(4, 2);
    _CLEAR_PIN(4, 3);
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

    clearpins();
    Wait(100);
}

void setdata(volatile int data) {
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

    clearpins();
    Wait(100);
}

// Only pass strings or char *
void writeMessage(char* message) {
    unsigned int i;
    for(i = 0; i < strlen(message); i++)
        setdata(message[i]);
}

void initLCD() {
    Wait(1000);
    setcmd(_FUNCTION_SET);
    setcmd(_DISPLAY_ON);
    setcmd(_DISPLAY_CLEAR);
    setcmd(_ENTRY_MODE);
}

void setInterrupt() {
    // Interrupt ports
    P1REN |= (BIT2 | BIT5 | BIT6);      // Enable resistor
    P1OUT |= (BIT2 | BIT5 | BIT6);
    P1IES |= (BIT2 | BIT5 | BIT6);      // Set interrupt edge
    P1IE |= (BIT2 | BIT5 | BIT6);       // Enable pin interrupt
    P1IFG &= ~(BIT2 | BIT5 | BIT6);     // Clear interrupt flag

    _BIS_SR(GIE);                       // Enable global interrupts
}

// ISR KEYPAD FINISH IMPLEMENTATION, MOVE FUNCTION CALLS ?????
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
    PUSH_FLAG = 1;
    counter++;

    // First Column
    if(P1IFG & BIT2) {
        // 1 - 4 Rows
        if(P10OUT & BIT3) {
            setdata('1');
        }

        if(P10OUT & BIT4) {
            setdata('4');
        }

        if(P10OUT & BIT5) {
            setdata('7');
        }

        if(P10OUT & BIT6) {
            setcmd(_DISPLAY_CLEAR);
        }
    }

    // Second Column
    if(P1IFG & BIT5) {
        // 1 - 4 Rows
        if(P10OUT & BIT3) {
            setdata('2');
        }

        if(P10OUT & BIT4) {
            setdata('5');
        }

        if(P10OUT & BIT5) {
            setdata('8');
        }

        if(P10OUT & BIT6) {
            setdata('0');
        }
    }

    // Third Column
    if(P1IFG & BIT6) {
        // 1 - 4 Rows
        if(P10OUT & BIT3) {
            setdata('3');
        }

        if(P10OUT & BIT4) {
            setdata('6');
        }

        if(P10OUT & BIT5) {
            setdata('9');
        }

        if(P10OUT & BIT6) {
            // Change line to write
        }
    }

    P1IFG &= ~(BIT2 | BIT5 | BIT6);
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer
    setLCDPorts();
    setKeypadPorts();
    setInterrupt();
    initLCD();

    // Loop rows and display counter
    while(1) {
        if(PUSH_FLAG == 1) {
            updateCountStr();
            // writeMessage(countStr);      // Uncomment to display counter in LCD ******
            PUSH_FLAG == 0;
        }


        P10OUT ^= BIT3;
        Wait(10);

        P10OUT ^= BIT3;
        P10OUT ^= BIT4;
        Wait(10);

        P10OUT ^= BIT4;
        P10OUT ^= BIT5;
        Wait(10);

        P10OUT ^= BIT5;
        P10OUT ^= BIT6;
        Wait(10);

        P10OUT ^= BIT6;
    }
}
