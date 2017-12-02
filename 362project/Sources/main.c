/*
************************************************************************
 ECE 362 - Mini-Project C Source File - Spring 2017
***********************************************************************
	 	   			 		  			 		  		
 Team ID: < ? >

 Project Name: < ? >

 Team Members:

   - Team/Doc Leader: < ? >      Signature: ______________________
   
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
void testLCD(char);
void pmsgterm(char[]);
int dispmenu(void);
int checkflex(void);
void array_maker(void);
void PrintCharacter(char* print , int a , int b);

/* Variable declarations */
int atd0 = 0;
int atd1 = 0;
int atd2 = 0;
int atd3 = 0;
int atd4 = 0;
int flex[5] = {0,0,0,0,0};
int tenthsec = 0;  // one-tenth second flag
int leftpb = 0;    // left pushbutton flag
int rghtpb = 0;    // right pushbutton flag
int runstp = 0;    // run/stop flag                         
int rticnt = 0;    // RTICNT (variable)
int prevpbleft = 0;    // previous state of left pushbuttons (variable)
int prevpbright = 0;	 	 // previous state of right pb  	   			 		  			 		       
int THRESH = 200;  
int zerorticnt = 0;
int menuItemCounter = 0;
unsigned char lcd_array[4][20];
char ADD = 0x80;
char pointer = 0;
int prevFlex[5] = {0,0,0,0,0};
int flexChange[5] = {0,0,0,0,0};
int timinterruptCounter = 0;
int checkCondition = 0;
int currentLocation = 0;
int currentRow = 0;
int currentCol = 0;
// LCD MENU
char *menu[5] = {"test0","test1","test2","test3","test4"};
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
#define TWOLINE 0x3C	// LCD 2-line enable command
#define CURMOV 0xFE	// LCD cursor move instruction
#define LINE1 0x80	// LCD line 1 cursor position
#define LINE2 0xC0	// LCD line 2 cursor position
#define LINE3 0x94	// LCD line 3 cursor position
#define LINE4 0xD4	// LCD line 4 cursor position

	 	   		
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
  ATDCTL3 = 0x28;
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
   send_i(0x30);
   send_i(0x30);
   send_i(0x30);
   send_i(TWOLINE);
   send_i(0x10);
   send_i(0x0C);
   send_i(0x06);
   send_i(LCDCLR);
   lcdwait();
   array_maker();
/* Initialize interrupts */

// RTI/interrupt initializations
    
     CRGINT = 0x80;
     RTICTL = 0x64;
     DDRT = 0xFF;
     PTT_PTT0 = 0;
     PTT_PTT1 = 0;

// TIM interrupt
   TSCR1 = 0x80; 
  TIOS = 0x80; 
  TSCR2 = 0x0C; 
  TC7 = 15000;
  TIE_C7I = 0;   
	      	      
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
	TIE_C7I = 1; 
  //dispmenu();
  PrintCharacter("Team 15",2,6);
  send_i(LCDCLR);
 for(;;) {
  
/* < start of your main loop > */ 
 if(tenthsec){
     tenthsec = 0;
     checkCondition = checkflex();
     if(checkCondition == 0){
      continue;
     } else {
      checkCondition = dispmenu();
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
 	timinterruptCounter++;
 	if(timinterruptCounter == 650) {
 	  if(flexChange[0] != 0) {
 	  flexChange[0] = 0;
   	}
   	if(flexChange[1] != 0) {
   	  flexChange[1] = 0;
   	}
   	if(flexChange[2] != 0) {
   	  flexChange[2] = 0;
   	}
   	if(flexChange[3] != 0) {
   	  flexChange[3] = 0 ;
   	}
   	if(flexChange[4] != 0) {
   	  flexChange[4] = 0 ;
   	}
   	timinterruptCounter = 0;
 	}

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
        if(x == LCDCLR) {
          currentLocation = 0; 
        }
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
  if(currentLocation > 80) {
    currentLocation = 0;
  }
  currentLocation += 1;
  currentRow = currentLocation / 20;
  currentCol = currentLocation % 20;
  if(currentCol == 19) {
    if(currentRow == 0){
      chgline(LINE2);
    }
    if(currentRow == 1){
      chgline(LINE3);
    }
    if(currentRow == 2){
      chgline(LINE4);
    }
    if(currentRow == 3){
      send_i(LCDCLR);
    }
  }
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

/*
***********************************************************************
 Name:         testLCD(
 Description:  Prints a character and waits one second before clearing
 Example:      testLCD('x');
***********************************************************************
*/

void testLCD(char test){
  
  int waitDisp = 0;
  send_i(LCDCLR);
  print_c(test);
  for (waitDisp = 0; waitDisp < 500; waitDisp += 1)
    lcdwait();
  send_i(LCDCLR);
}

/*
***********************************************************************
  pmsgterm: print character string str[] on terminal
***********************************************************************
*/
void pmsgterm(char str[]){

    int counter = 0;
  while(str[counter] != '\0'){
    outchar(str[counter]);
    counter++;
  }
}

/*
***********************************************************************
  dispmenu: displays and controls menu on the LCD
***********************************************************************
*/
int dispmenu(){
  int waitDisp = 0;
  for(;;) {
    send_i(LCDCLR);
    pmsglcd(menu[menuItemCounter]);
    pmsgterm(menu[menuItemCounter]);
    //---- 1 Second wait----------
    for (waitDisp = 0; waitDisp < 500; waitDisp += 1)
      lcdwait();
    //------1 Second Wait-----------
    checkflex();
    if(flex[3] == 1) {
      return 0;
    }
    
    if(flex[1] == 1) {
      if(menuItemCounter == 0)
        menuItemCounter = 4;
      else
        menuItemCounter--;
    }
      
    if(flex[2] == 1) {
        if(menuItemCounter == 4)
          menuItemCounter = 0;
        else
          menuItemCounter++;
    }
  }
  return 0;
}

/*
***********************************************************************
  checkflex: does the atd conversion 
***********************************************************************
*/

int checkflex() {
      
      flex[0] = 0;
      flex[1] = 0;
      flex[2] = 0;
      flex[3] = 0;
      flex[4] = 0;
      
      ATDCTL5 = 0x10;
      while(!(ATDSTAT0 & 0x80)){}
      atd0 = 255 - ATDDR0H;
      atd1 = 255 - ATDDR1H;
      atd2 = 255 - ATDDR2H;
      atd3 = 255 - ATDDR3H;
      atd4 = 255 - ATDDR4H;
      
        if(atd0 > THRESH){
          flex[0] = 1;
          zerorticnt = 0;
        }
        if(atd1 > THRESH){
          flex[1] = 1;
          zerorticnt = 0;
        }
        if(atd2 > THRESH){
          flex[2] = 1;
          zerorticnt = 0;
        }
        if(atd3 > THRESH){
          flex[3] = 1;
          zerorticnt = 0;
        }
        if(atd4 > THRESH){
          flex[4] = 1;
          zerorticnt = 0;
        }
        
        if(flex[0] == 1 && prevFlex[0] != flex[0]) {
          flexChange[0] += 1;
          if(flexChange[0] == 1) {
           pmsglcd("a"); 
          }
          if(flexChange[0] == 2) {
           pmsglcd("b"); 
          }
          if(flexChange[0] == 3) {
           pmsglcd("c"); 
          }
          if(flexChange[0] == 4) {
           pmsglcd("d"); 
          }
          if(flexChange[0] == 5) {
           pmsglcd("e"); 
          }
        }
        if(flex[1] == 1 && prevFlex[1] != flex[1]) {
          flexChange[1] += 1;
         if(flexChange[1] == 1) {
           pmsglcd("f"); 
          }
          if(flexChange[1] == 2) {
           pmsglcd("g"); 
          }
          if(flexChange[1] == 3) {
           pmsglcd("h"); 
          }
          if(flexChange[1] == 4) {
           pmsglcd("i"); 
          }
          if(flexChange[1] == 5) {
           pmsglcd("j"); 
          }
          
        }
        if(flex[2] == 1 && prevFlex[2] != flex[2]) {
          flexChange[2] += 1;
         if(flexChange[2] == 1) {
           pmsglcd("k"); 
          }
          if(flexChange[2] == 2) {
           pmsglcd("l"); 
          }
          if(flexChange[2] == 3) {
           pmsglcd("m"); 
          }
          if(flexChange[2] == 4) {
           pmsglcd("n"); 
          }
          if(flexChange[2] == 5) {
           pmsglcd("o"); 
          }
          
        }
        if(flex[3] == 1 && prevFlex[3] != flex[3]) {
          flexChange[3] += 1;
          if(flexChange[3] == 1) {
           pmsglcd("p"); 
          }
          if(flexChange[3] == 2) {
           pmsglcd("q"); 
          }
          if(flexChange[3] == 3) {
           pmsglcd("r"); 
          }
          if(flexChange[3] == 4) {
           pmsglcd("s"); 
          }
          if(flexChange[3] == 5) {
           pmsglcd("t"); 
          }
        }
        if(flex[4] == 1 && prevFlex[4] != flex[4]) {
          flexChange[4] += 1;
        if(flexChange[4] == 1) {
           pmsglcd("u"); 
          }
          if(flexChange[4] == 2) {
           pmsglcd("v"); 
          }
          if(flexChange[4] == 3) {
           pmsglcd("w"); 
          }
          if(flexChange[4] == 4) {
           pmsglcd("x"); 
          }
          if(flexChange[4] == 5) {
           pmsglcd("y"); 
          }
          if(flexChange[4] == 6) {
           pmsglcd("z"); 
          }
          
        }
        
        prevFlex[0] = flex[0];
        prevFlex[1] = flex[1];
        prevFlex[2] = flex[2];
        prevFlex[3] = flex[3];
        prevFlex[4] = flex[4];
        if(flexChange[1] != 0 && flexChange[2] != 0) {
          return 12;
        }
        
         
   	    if(flexChange[0] == 5) {
 	        flexChange[0] = 0;
   	    }
   	    if(flexChange[1] == 5) {
   	      flexChange[1] = 0;
   	    }
   	    if(flexChange[2] == 5) {
   	      flexChange[2] = 0;
   	    }
   	    if(flexChange[3] == 5) {
   	      flexChange[3] = 0 ;
   	    }
   	    if(flexChange[4] == 6) {
   	      flexChange[4] = 0 ;
   	    }
   	    
   	    if(flexChange[0] != 0 && flexChange[4] != 0) {
   	      send_i(LCDCLR); 
   	    }
   	    
   	    if(flexChange[1] != 0 && flexChange[4] != 0) {
   	      currentLocation -= 1;
   	      pmsglcd(""); 
   	    }
    return 0;
        
}
/*****************************************************************************************
 * Function: Array maker
 *
 * This function is used to covert the lcd display into a  4 , 20 matrix
 * so that the user can enter only the coordinates of where he wants to insert
 * information
 *
 ****************************************************************************************/
void array_maker(void){
	int i , j;
	i = 0;
	j = 0;
	for( j = 0 ; j < 20 ; j++){
		lcd_array [i] [j] = ADD ;
		ADD = ADD + 1;
	}
	i = 1 ;
	ADD = 0xc0;
	for( j = 0 ; j < 20 ; j++){
		lcd_array [i] [j] = ADD ;
		ADD = ADD + 0x01;
	}
	i = 2;
	ADD = 0x94;
	j = 0;
	lcd_array [i] [j] = ADD ;
	for( j = 0 ; j < 20 ; j++){
		lcd_array [i] [j] = ADD ;
		ADD = ADD + 0x01;
	}
	i = 3;
	ADD = 0xD4;
	j = 0;
	lcd_array [i] [j] = ADD ;
	for( j = 0 ; j < 20 ; j++){
		lcd_array [i] [j] = ADD ;
		ADD = ADD + 0x01;
	}
}
/*****************************************************************************************
 * Function: PrintCharacter
 *
 *	This function is used to print a string to the lcd display and
 *	you need to provide the x and y location
 *
 *
 ****************************************************************************************/
void PrintCharacter(char* print , int a , int b){
	int i = 0;
	a = a - 1;
	b = b - 1;
	pointer = lcd_array [a][b];
	send_i(pointer);
	while(print[i] != '\0'){
		print_c(print[i]);
		i++;
	}
}
