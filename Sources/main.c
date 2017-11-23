/*
************************************************************************
 ECE 362 - Mini-Project C Source File - Spring 2017
***********************************************************************
	 	   			 		  			 		  		
 Team ID: < 15 >

 Project Name: < ? >

 Team Members:

   - Team/Doc Leader: < Devashish >      Signature: ______________________
   
   - Software Leader: < ? >      Signature: ______________________

   - Interface Leader: < ? >     Signature: ______________________

   - Peripheral Leader: < ? >    Signature: ______________________


 Academic Honesty Statement:  In signing above, we hereby certify that we 
 are the individuals who created this HC(S)12 source file and that we have
 not copied the work of any other student (past or present) while completing 
 it. We understand that if we fail to honor this agreement, we will receive 
 a grade of ZERO and be subject to possible disciplinary action.

***********************************************************************

 The objective of this Mini-Project is to .... < ? >


***********************************************************************

 List of project-specific success criteria (functionality that will be
 demonstrated):

 1.

 2.

 3.

 4.

 5.

***********************************************************************

  Date code started: < ? >

  Update history (add an entry every time a significant change is made):

  Date: < ? >  Name: < ? >   Update: < ? >

  Date: < ? >  Name: < ? >   Update: < ? >

  Date: < ? >  Name: < ? >   Update: < ? >


***********************************************************************
*/

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

/* All functions after main should be initialized here */
char inchar(void);
void outchar(char x);
void shiftout(char); // LCD drivers (written previously)
void lcdwait(void);
void send_byte(char);
void send_i(char);
void chgline(char);
void print_c(char);
void pmsglcd(char[]);


/* Variable declarations */
int atd1 = 0;
int tenthsec = 0;  // one-tenth second flag
int leftpb = 0;    // left pushbutton flag
int rghtpb = 0;    // right pushbutton flag
int runstp = 0;    // run/stop flag                         
int rticnt = 0;    // RTICNT (variable)
int prevpbleft = 0;    // previous state of left pushbuttons (variable)
int prevpbright = 0;	 	 // previous state of right pb  	   			 		  			 		       
int THRESH = 200;  
int zerorticnt = 0;

/* Special ASCII characters */
#define CR 0x0D		// ASCII return 
#define LF 0x0A		// ASCII new line 

/* LCD COMMUNICATION BIT MASKS (note - different than previous labs) */
#define RS 0x10		// RS pin mask (PTT[4])
#define RW 0x20		// R/W pin mask (PTT[5])
#define LCDCLK 0x40	// LCD EN/CLK pin mask (PTT[6])

/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0F	// LCD initialization command
#define LCDCLR 0x01	// LCD clear display command
#define TWOLINE 0x38	// LCD 2-line enable command
#define CURMOV 0xFE	// LCD cursor move instruction
#define LINE1 = 0x80	// LCD line 1 cursor position
#define LINE2 = 0xC0	// LCD line 2 cursor position

	 	   		
/*	 	   		
***********************************************************************
 Initializations
***********************************************************************
*/

void  initializations(void) {

/* Set the PLL speed (bus clock = 24 MHz) */
  CLKSEL = CLKSEL & 0x80; //; disengage PLL from system
  PLLCTL = PLLCTL | 0x40; //; turn on PLL
  SYNR = 0x02;            //; set PLL multiplier
  REFDV = 0;              //; set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; //; engage PLL

/* Disable watchdog timer (COPCTL register) */
  COPCTL = 0x40   ; //COP off; RTI and COP stopped in BDM-mode

/* Initialize asynchronous serial port (SCI) for 9600 baud, interrupts off initially */
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port

/* Initialize peripherals */

// ATD initialization

//  Initialize Port AD pins 7 and 6 for use as digital inputs

  DDRT = 0xFF; 
	DDRAD = 0; 		//program port AD for input mode
  ATDDIEN = 0xC0; //program PAD7 and PAD6 pins as digital inputs
  ATDCTL2 = 0x80;
  ATDCTL3 = 0x08;
  ATDCTL4 = 0x85;
  
/*
Initialize SPI for baud rate of 6 Mbs, MSB first
  (note that R/S, R/W', and LCD clk are on different PTT pins)
*/
   PTM = 0;
   DDRM = 0x30;
   SPICR1 = 0x50;
   SPICR2 = 0x00;
   SPIBR = 0x01;


// LCD initializations
   PTT_PTT6 = 1;
   PTT_PTT5 = 0;
   send_i(LCDON);
   send_i(TWOLINE);
   send_i(LCDCLR);
   lcdwait();
   
/* Initialize interrupts */

// RTI/interrupt initializations
    
     CRGINT = 0x80;
     RTICTL = 0x64;
     DDRT = 0xFF;
     PTT_PTT0 = 0;
     PTT_PTT1 = 0;
	      
	      
}

	 		  			 		  		
/*	 		  			 		  		
***********************************************************************
Main
***********************************************************************
*/
void main(void) {
  int waitDisp = 0;
  DisableInterrupts
	initializations(); 		  			 		  		
	EnableInterrupts;
	
 for(;;) {
  
/* < start of your main loop > */ 
 if(tenthsec){
     tenthsec = 0;
      ATDCTL5 = 0x10;
      while(!(ATDSTAT0 & 0x80)){}
      atd1 = 255 - ATDDR0H;  
        if(atd1 > THRESH){
           // -------------------------- TESTING LCD -----------------------------------
          /*
          send_i(LCDCLR);
          pmsglcd("1");
          for (waitDisp = 0; waitDisp < 500; waitDisp += 1) {
            lcdwait();
            send_i(LCDCLR);
          }*/
           // -------------------------- TESTING LCD -----------------------------------
          
          outchar('1');
          zerorticnt = 0;
        }else if(zerorticnt > 30){
          
          // -------------------------- TESTING LCD -----------------------------------
          /*
          send_i(LCDCLR);
          pmsglcd("0");
          for (waitDisp = 0; waitDisp < 500; waitDisp += 1) {
            lcdwait();
            send_i(LCDCLR);
          }*/
          // ---------------------------------------------------------------------------          
          outchar('0');
          zerorticnt = 0;
        }else{
        }
 }
      

  if(leftpb == 1) {
    leftpb = 0;
    runstp = 0;
  }
  
  if(rghtpb == 1) {
    rghtpb = 0;
    runstp = 1;
  }
  
 
 }/* loop forever */
   
}/* do not leave main */




/*
***********************************************************************                       
 RTI interrupt service routine: RTI_ISR
************************************************************************
*/

interrupt 7 void RTI_ISR(void) {
  
  	// clear RTI interrupt flagt 
  	CRGFLG = CRGFLG | 0x80;
  	rticnt += 1;
	   if(rticnt == 10) {
	    rticnt = 0;
	    tenthsec = 1;
	   }
    if ((prevpbleft == 1) &&(PORTAD0_PTAD7 == 0)){
      leftpb = 1; 
    }
    if ((prevpbright == 1) &&(PORTAD0_PTAD6 == 0)){
      rghtpb = 1; 
    }
    
    zerorticnt += 1;
    prevpbleft = PORTAD0_PTAD7;
    prevpbright = PORTAD0_PTAD6;
}

/*
***********************************************************************                       
  TIM interrupt service routine	  		
***********************************************************************
*/

interrupt 15 void TIM_ISR(void)
{
  	// clear TIM CH 7 interrupt flag 
 	TFLG1 = TFLG1 | 0x80; 
 

}

/*
***********************************************************************                       
  SCI interrupt service routine		 		  		
***********************************************************************
*/

interrupt 20 void SCI_ISR(void)
{
 


}


/*
***********************************************************************
  shiftout: Transmits the character x to external shift
            register using the SPI.  It should shift MSB first.
            MISO = PM[4]
            SCK  = PM[5]
***********************************************************************
*/

void shiftout(char x)

{

  // test the SPTEF bit: wait if 0; else, continue
  // write data x to SPI data register
  // wait for 30 cycles for SPI data to shift out
   int readBit;
  while(SPISR_SPTEF == 0){}
  SPIDR = x;
  for(readBit = 0; readBit < 30; readBit++){}

}

/*
***********************************************************************
  lcdwait: Delay for approx 2 ms
***********************************************************************
*/

void lcdwait()
{
   int wait;
  for(wait = 0; wait < 7999; wait++){}
}

/*
***********************************************************************
  send_byte: writes character x to the LCD
***********************************************************************
*/

void send_byte(char x)
{
     // shift out character
     // pulse LCD clock line low->high->low
     // wait 2 ms for LCD to process data
     shiftout(x);
    PTT_PTT6 = 0;
    PTT_PTT6 = 1;
    PTT_PTT6 = 0;
    lcdwait();

}

/*
***********************************************************************
  send_i: Sends instruction byte x to LCD
***********************************************************************
*/

void send_i(char x)
{
        // set the register select line low (instruction data)
        // send byte
         PTT_PTT4 = 0;
        send_byte(x);
}

/*
***********************************************************************
  chgline: Move LCD cursor to position x
  NOTE: Cursor positions are encoded in the LINE1/LINE2 variables
***********************************************************************
*/

void chgline(char x)
{
   send_i(CURMOV);
  send_i(x);
}

/*
***********************************************************************
  print_c: Print (single) character x on LCD
***********************************************************************
*/

void print_c(char x)
{
    PTT_PTT4 = 1;
  send_byte(x);
}

/*
***********************************************************************
  pmsglcd: print character string str[] on LCD
***********************************************************************
*/

void pmsglcd(char str[])
{
    int counter =0;
  while(str[counter] != '\0'){
    print_c(str[counter]);
    counter++;
  }
}




/*
***********************************************************************
 Character I/O Library Routines for 9S12C32 
***********************************************************************
 Name:         inchar
 Description:  inputs ASCII character from SCI serial port and returns it
 Example:      char ch1 = inchar();
***********************************************************************
*/

char inchar(void) {
  /* receives character from the terminal channel */
        while (!(SCISR1 & 0x20)); /* wait for input */
    return SCIDRL;
}

/*
***********************************************************************
 Name:         outchar    (use only for DEBUGGING purposes)
 Description:  outputs ASCII character x to SCI serial port
 Example:      outchar('x');
***********************************************************************
*/

void outchar(char x) {
  /* sends a character to the terminal channel */
    while (!(SCISR1 & 0x80));  /* wait for output buffer empty */
    SCIDRL = x;
}


