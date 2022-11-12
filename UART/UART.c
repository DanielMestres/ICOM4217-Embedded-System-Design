#include <msp430.h>
/*
* Macros
*/
#define _SET_OUTPUT(port, pin)          P ## port ## DIR |= BIT ## pin
#define _SET_INPUT(port, pin)           P ## port ## DIR &= ~BIT ## pin
#define _SET_PIN(port, pin)             P ## port ## OUT |= BIT ## pin
#define _CLEAR_PIN(port, pin)           P ## port ## OUT &= ~BIT ## pin

#define _FUNCTION_SET                   0x3B
#define _DISPLAY_ON                     0x0D
#define _DISPLAY_CLEAR                  0x01
#define _ENTRY_MODE                     0x06
#define _CURSOR_LEFT                    0x10
#define _CURSOR_RIGHT                   0x14

#define _SECONDS                        0x00
#define _MINUTES                        0x01
#define _HOURS                          0x02
#define _DAY                            0x03
#define _DATE                           0x04
#define _MONTH                          0x05
#define _YEAR                           0x06

/*
* Reference
* http://www.simplyembedded.org/tutorials/msp430-uart/
*/

/*
* Prototypes
*/
static void Wait(volatile int time_1);
char toUpper(char character);
void setPorts(void);
void setUART(void);
void setI2C(void);
void receive(void);
void activateInterrupts();
void setWriteInstruction(void);
void setWriteData(void);
void enable(void);
void setcmd(int num);
void setdata(int data);
void setLine(void);
void writeLCD(char* word);
void LCDinit(void);
void writeBuffer(char* message);

/*
* Globals
*/
char letter;
unsigned char I2CData[8];

/**
* main.c
*/
void main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Stop Watchdog Timer
    setPorts();
    LCDinit();
     setUART();
//    setI2C();
    activateInterrupts();

    // First Exercise
    writeBuffer("Hello World!");
//    setLine();
    while(1) {
//        receive();
    }
}

/*
* Implementation
*/
static void Wait(volatile int time_1) {
    volatile int time_2;
    for (time_1; time_1>1; time_1--) {
        for (time_2 = 110; time_2 >= 0; time_2--);
    }
}

// ONLY LOWERCASE LETTERS
char toUpper(char character) {
    if(character != ' ' || (character >= 97 && character <= 122)) {
        return character - 32;
    } else {
        return character;
    }
}

void setPorts() {
    // LCD
    _SET_OUTPUT(10, 0);     // BIT0
    _SET_OUTPUT(10, 1);     // BIT1
    _SET_OUTPUT(10, 2);     // BIT2
    _SET_OUTPUT(10, 3);     // BIT3
    _SET_OUTPUT(10, 4);     // BIT4
    _SET_OUTPUT(10, 5);     // BIT5
    _SET_OUTPUT(10, 6);     // BIT6
    _SET_OUTPUT(10, 7);     // BIT7
    _SET_OUTPUT(6, 7);      // RS
    _SET_OUTPUT(7, 4);      // RW
    _SET_OUTPUT(7, 5);      // E

    // UART
    _SET_OUTPUT(3, 4);      // TX
    _SET_INPUT(3, 5);       // RX

    // I2C
    _SET_OUTPUT(3, 2);      // SCL
    _SET_INPUT(3, 1);       // SDA
}

void setUART() {
    P3SEL |= (BIT4 | BIT5);         // PINS 3.4,5 = USCI_A0 TXD / RXD
    UCA0CTL1 |= UCSWRST;            // Standby UART
    UCA0CTL1 |= UCSSEL_2;           // SMCLK 1MHz
    UCA0MCTL &= ~UCOS16;            // ucos16 == 0
    UCA0BRW = 105;                  // UCBRx Baud Rate Control Register 0
    // UCA0MCTL |= (UCBRS1 | UCBRF0);  // Modulation
    UCA0CTL0 |= UCSPB;              // Enabling Receiver
    UCA0CTL1 &= ~UCSWRST;           // Start UART

    UCA0IE |= UCRXIE;               // Initialize RX interrupt
}

void setI2C() {
    P3SEL |= (BIT1 | BIT2);
    UCB0CTL1 |= UCSWRST;            // Needed to modify CTL0 and CTL1
    UCB0CTL0 |= (UCMODE_3 | UCMST | UCSYNC); // I2C mode | master mode | Sync Mode
    UCB0CTL1 |= UCSSEL_2;           // smclk
    //UCB0BRW = 0x64;               // 10k baud rate, smclk / 0x64 (100)
    UCB0BR0 = 100;
    UCB0BR1 = 0;
    UCB0I2CSA = 0x68;               // set slave address (0x68 / 0xD0 write / 0xD1 read)
    //UCB0CTL1 |= UCTXSTT;          // Transmit start condition
    UCB0CTL1 &= ~UCSWRST;           // Initialize USCI state machine
    UCB0IE |= (UCRXIE);             // Enable RX interrupt
}

void receive() {
    UCB0CTL1 &= ~UCTR; // Receiver Mode
    UCB0CTL1 |= UCTXSTT; // Start Condition
    while(UCB0CTL1 & UCTXSTT); // Waits for slave to acknowledge at specified address
}

void activateInterrupts() {
    _BIS_SR(GIE); // Enable Global Interrupts + LPM (LPM#_bits)
}

void setWriteInstruction() {
    _CLEAR_PIN(6, 7); // RS output (0)
    _CLEAR_PIN(7, 4); // RW output (0)
}

void setWriteData() {
    _SET_PIN(6, 7); // RS output (1)
    _CLEAR_PIN(7, 4); // RW output (0)
}

void enable() {
    _SET_PIN(7, 5); // E output (1)
    _CLEAR_PIN(7, 5); // E output (0)
}

void setcmd(int num) {
    setWriteInstruction();
    P10OUT = num;
    enable();
    Wait(5);
}

void setdata(int data) {
    setWriteData();
    P10OUT = data;
    enable();
    Wait(5);
}

void writeLCD(char* word) {
    unsigned int index;
    for(index = 0; index < strlen(word); index++) {
        setdata(word[index]);
    }
}

void setLine() {
    setWriteInstruction();
    int i;
    // command = 0x10
    for(i = 0; i < 40; i++) {
       setcmd(_CURSOR_RIGHT);
    }
}

void LCDinit() {
    Wait(1000);
    setcmd(_FUNCTION_SET);
    setcmd(_DISPLAY_ON);
    setcmd(_DISPLAY_CLEAR);
    setcmd(_ENTRY_MODE);
}

void writeBuffer(char* message){
    unsigned int index;
    for(index = 0; index < strlen(message); index++) {
        while(!(UCA0IFG & UCTXIFG)); // Wait for buffer to be empty (Polling)
        UCA0TXBUF = message[index];
    }
}

/*
* ISR's
*/

// Second Exercise
#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void) {
    letter = UCA0RXBUF;
    switch(letter) {
        case '~':
            setcmd(_DISPLAY_CLEAR);
            break;
        case '+':
            setLine();
            break;
        default:
            setdata(letter);
            break;
    }
//    UCA0TXBUF = toUpper(letter);         // Echo back to PUTTY (UPPERCASE)
    UCA0TXBUF = letter;                 // Echo back to PUTTY
    UCA0IFG &= ~UCRXIFG;
}

// Third Exercise
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void) {
//    setdata(UCB0RXBUF);
}
