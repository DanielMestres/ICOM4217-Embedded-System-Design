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
char* prntnum(unsigned long num, int base, char sign, char *outbuf);
void hextobin(char* hex);
void setPorts(void);
void setInterrupts(void);
void setWriteInstruction(void);
void setWriteData(void);
void enable(void);
void setcmd(int num);
void setdata(int data);
void setLine(void);
void writeLCD(char* word);
void LCDinit(void);
void inputDAC(int num);
void setADC(void);
void setTimer(void);

/*
 * Globals
 */
int values[12] = {0x00, 0x17, 0x2E, 0x45, 0x5C, 0x73, 0x8A, 0xA1, 0xB8, 0xCF, 0xE6, 0xFF};
char* valuesString[12] = {"00", "17", "2E", "45", "5C", "73", "8A", "A1", "B8", "CF", "E6", "FF"};
unsigned int index = 0;
//char hex[12];

/*
 * main.c
 */
void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	setPorts();
	LCDinit();
	setADC();

	setTimer();
	setInterrupts();

	while(1) {
	    ADC12CTL0 |= ADC12SC; // Start sampling/conversion
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

// Second exercise
char* prntnum(unsigned long num, int base, char sign, char *outbuf)
{

    int i = 12;
    int j = 0;

    do{
        outbuf[i] = "0123456789ABCDEF"[num % base];
        i--;
        num = num/base;
    }while( num > 0);

    if(sign == '-'){
        outbuf[0] = sign;
        ++j;
    }

    while( ++i < 13){
       outbuf[j++] = outbuf[i];
    }

    outbuf[j] = 0;

    return outbuf;

//    setcmd(_DISPLAY_CLEAR);
//    writeLCD(hex);
//    setLine();
//    writeLCD("v= ");

}

// Print binary value to LCD
void hextobin(char* hex) {
   int i=0;
   setcmd(_DISPLAY_CLEAR);
   while(hex[i])
   {
       switch(hex[i])
       {
           case '0':
               writeLCD("0000");
           break;
           case '1':
               writeLCD("0001");
           break;
           case '2':
               writeLCD("0010");
           break;
           case '3':
               writeLCD("0011");
           break;
           case '4':
               writeLCD("0100");
           break;
           case '5':
               writeLCD("0101");
           break;
           case '6':
               writeLCD("0110");
           break;
           case '7':
               writeLCD("0111");
           break;
           case '8':
               writeLCD("1000");
           break;
           case '9':
               writeLCD("1000");
           break;
           case 'A':
               writeLCD("1010");
           break;
           case 'a':
               writeLCD("1010");
           break;
           case 'B':
               writeLCD("1011");
           break;
           case 'b':
               writeLCD("1011");
           break;
           case 'C':
               writeLCD("1100");
           break;
           case 'c':
               writeLCD("1100");
           break;
           case 'D':
               writeLCD("1101");
           break;
           case 'd':
               writeLCD("1101");
           break;
           case 'E':
               writeLCD("1110");
           break;
           case 'e':
               writeLCD("1110");
           break;
           case 'F':
               writeLCD("1111");
           break;
           case 'f':
               writeLCD("1111");
           break;
       }
       i++;
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
    _SET_OUTPUT(7, 7);      // RS
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
    _SET_INPUT(6, 7);       // A7 analog input
}

void setInterrupts() {
    _BIS_SR(GIE);           // Enable global interrupts
}

void setWriteInstruction() {
    _CLEAR_PIN(7, 7);       // RS output (0)
    _CLEAR_PIN(7, 4);       // RW output (0)
}

void setWriteData() {
    _SET_PIN(7, 7);         // RS output (1)
    _CLEAR_PIN(7, 4);       // RW output (0)
}

void enable() {
    _SET_PIN(7, 5);         // E output (1)
    _CLEAR_PIN(7, 5);       // E output (0)
}

void setcmd(int num) {
    setWriteInstruction();
    P10OUT = num;
    enable();
    Wait(3);
}

void setdata(int data) {
    setWriteData();
    P10OUT = data;
    enable();
    Wait(3);
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

void inputDAC(int num) {
    P3OUT = num;
}

void setTimer() {
    //Timer0_A0 Configuration
    TA0CCTL0 |= CCIE;                   // Enable interrupt for CCR0.
    TA0CTL = (TASSEL_1 | ID_0 | MC_1);  // ACLK, ACLK/1, Up Mode
    TA0CCR0 = 39321;                    // Compare Register, f = 32768 / (TA0CCR) f = 1Hz
//    TA0CCR0 = 65;                    // Compare Register, f = 32768 / (TA0CCR) f = 500Hz
    TA0CTL &= ~TAIFG;
}

void setADC() {
    P6SEL |= BIT7;                                          // Select 6.7 A7
    ADC12CTL0 &= ~ADC12ENC;                                 // necessary to setup registers

    ADC12CTL0 = (ADC12SHT0_1 | ADC12ON | ADC12MSC);         // 8  ADC12CLK cycles, adc12 on

    ADC12CTL1 |= (ADC12SHP | ADC12SSEL_0 | ADC12CONSEQ_2);  // sampling timer
    ADC12CTL2 |= ADC12RES_2;                                // set 12bit resolution(max)
    ADC12MCTL0 = (ADC12SREF_0 | ADC12INCH_7);               // MEM0 for A7, PIN 6.7 input

    ADC12CTL0 |= ADC12ENC;                                  // enable conversion
//    ADC12IE = ADC12IE0;                                     // Interrupt for MEM0
}

/*
 * ISR's
 */
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void)
{
//    inputDAC(values[index]);
//    hextobin(valuesString[index]);
//    writeLCD(values[index]);

//    if(index != 11) {
//        index++;
//    }else {
//        index = 0;
//    }
    int decimal = ADC12MEM0;
    char* hex = malloc(sizeof(char*));
    hex = prntnum(decimal, 16, '+', hex);
    setcmd(_DISPLAY_CLEAR);
    writeLCD(hex);
    setLine();
    writeLCD("v= ");
    free(hex);

    TA0CTL &= ~TAIFG;
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void) {
//    writeLCD("Interrupt!");
//    setcmd(_DISPLAY_CLEAR);
}
