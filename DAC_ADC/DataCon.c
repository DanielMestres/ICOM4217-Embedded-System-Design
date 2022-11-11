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
void writeLCD(char* word);
void displayDAC(int data);
void LCDinit(void);
void inputDAC(int num);
void setADC(void);
void setTimer(void);

/*
 * Globals
 */
int values[12] = {0x00, 0x17, 0x2E, 0x45, 0x5C, 0x73, 0x8A, 0xA1, 0xB8, 0xCF, 0xE6, 0xFF};
int index = 0;

/*
 * main.c
 */
void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	setPorts();
	LCDinit();
//	setADC();

	setTimer();
	setInterrupts();

	while(1) {

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
    _SET_OUTPUT(3, 6);      // RS
    _SET_OUTPUT(7, 4);      // RW
    _SET_OUTPUT(7, 5);      // E

    // DAC
    _SET_OUTPUT(3, 0);     // BIT0
    _SET_OUTPUT(3, 1);     // BIT1
    _SET_OUTPUT(3, 2);     // BIT2
    _SET_OUTPUT(3, 3);     // BIT3
    _SET_OUTPUT(3, 4);     // BIT4
    _SET_OUTPUT(3, 5);     // BIT5
    _SET_OUTPUT(3, 6);     // BIT6
    _SET_OUTPUT(3, 7);     // BIT7

    // ADC
//    _SET_INPUT(6, 7);       // A7 analog input
}

void setInterrupts() {
    _BIS_SR(GIE);                       // Enable global interrupts
}

void setWriteInstruction() {
    _CLEAR_PIN(3, 6); // RS output (0)
    _CLEAR_PIN(7, 4); // RW output (0)
}

void setWriteData() {
    _SET_PIN(3, 6); // RS output (1)
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

// Display binary in LCD
void displayDAC(int data) {

}

void LCDinit() {
    Wait(1000);
    setcmd(_FUNCTION_SET);
    setcmd(_DISPLAY_ON);
    setcmd(_DISPLAY_CLEAR);
    setcmd(_ENTRY_MODE);
}

void inputDAC(int num) {
    P3OUT = num;
}

void setTimer() {
    //Timer0_A0 Configuration
    TA0CCTL0 |= CCIE;                   // Enable interrupt for CCR0.
    TA0CTL = (TASSEL_1 | ID_0 | MC_1);  // ACLK, ACLK/1, Up Mode
    TA0CCR0 = 32768;                    // Compare Register, f = 32768 / (TA0CCR) f = 1Hz
    TA0CTL &= ~TAIFG;
}

// max = 4096, FIX
void setADC() {
    P6SEL |= BIT7;              // Select 6.7
    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 = ADC12SHT02 + ADC12ON; //sampling time, adc12 on
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_2 + ADC12SSEL_2 + ADC12SHS_3; //sampling timer, repeated sampling on single channel,
                                                                      //timer B trigger sampling
    ADC12MCTL0 = ADC12SREF_0 + ADC12INCH_7;      // A7, PIN 6.7
    ADC12CTL0 |= ADC12ENC;                       //enable conversion
    ADC12IE = 0x01;                              //Interrupt for MEM0
}

/*
 * ISR
 */
//Timer ISR
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void)
{
    inputDAC(values[index]);
    if(index < 12) {
        index++;
    }else {
        index = 0;
    }
    TA0CTL &= ~TAIFG;
}
