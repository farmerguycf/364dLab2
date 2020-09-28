/*
    Remove all unnecessary lines (including this one) 
    in this comment.
    REFER TO THE SUBMISSION INSTRUCTION FOR DETAILS

    Name 1: guy farmer 
    Name 2: tom li
    UTEID 1: gcf375
    UTEID 2: hl26868
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();
void and(int currInstruction);
void add(int currInstruction);
void br(int currInstruction);
void jmp(int currInstruction);
void jsrr(int currInstruction);
void ldb(int currInstruction);
void ldw(int currInstruction);
void lea(int currInstruction);
void not(int currInstruction);
void shf(int currInstruction);
void stb(int currInstruction);
void stw(int currInstruction);
void trap(int currInstruction);
void jsrr(int currInstruction);
void sext(int *number, int bit);
int isPositive(int num);
int isNegative(int num);
/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
  int address; /* this is a byte address */

  printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
  int k; 

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3b-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
 }
  CURRENT_LATCHES.Z = 1;  
  NEXT_LATCHES = CURRENT_LATCHES;
    
  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/



void process_instruction(){
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */   
    System_Latches currentLatch = CURRENT_LATCHES;

    // here is the fetch portion currInstruction is the current intruction  
    int currInstLow = MEMORY[CURRENT_LATCHES.PC>>1][0]&0x00ff;
    int currInstHigh = MEMORY[CURRENT_LATCHES.PC>>1][1]&0x00ff;
    currInstHigh = currInstHigh << 8;
    int currInstruction = currInstHigh | currInstLow;
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC+=2;
    NEXT_LATCHES.N = NEXT_LATCHES.P = NEXT_LATCHES.Z = 0;
    // fetch portion ended
    // 	here is the decode portion
    int opcode = currInstruction & 0xf000;
    if(opcode == 0x5000) and(currInstruction);
    if(opcode == 0x1000) add(currInstruction);
    if(opcode == 0x0000) br(currInstruction);
    if(opcode == 0xc000) jmp(currInstruction);
    if(opcode == 0x4000) jsrr(currInstruction);
    if(opcode == 0x2000) ldb(currInstruction);
    if(opcode == 0x6000) ldw(currInstruction);   
    if(opcode == 0xe000) lea(currInstruction);
    if(opcode == 0x9000) not(currInstruction);
    if(opcode == 0xd000) shf(currInstruction);
    if(opcode == 0x3000) stb(currInstruction);
    if(opcode == 0x7000) stw(currInstruction);
    if(opcode == 0xf000) trap(currInstruction);
}
void and(int currInstruction){

    //	if (bit[5] == 0)
    //	DR = SR1 AND SR2;
    //	else
    //	DR = SR1 AND SEXT(imm5);
    //	setcc();
    int destReg;
    int srcReg1;
    int srcReg2;
    int imm5;

    destReg = CURRENT_LATCHES.REGS[(currInstruction&0x0E00)>>9];
    srcReg1 = CURRENT_LATCHES.REGS[(currInstruction&0x01c0)>>6];
    srcReg2 = CURRENT_LATCHES.REGS[(currInstruction&0x0007)];

    imm5 = currInstruction&0x001f;
    sext(&imm5,4);
    if (!(currInstruction & 0x0020)){
        NEXT_LATCHES.REGS[(currInstruction&0x0E00)>>9] = Low16bits(srcReg1 & srcReg2);
    }
    else{
        NEXT_LATCHES.REGS[(currInstruction&0x0E00)>>9] = Low16bits(srcReg1 & imm5);
    }
    if(isPositive(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9])){NEXT_LATCHES.P = 1;}
    if(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9]==0){NEXT_LATCHES.Z = 1;}
    if(isNegative(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9])){NEXT_LATCHES.N = 1;}
    // requires testing
}
void add(int currInstruction){
    int destReg;
    int srcReg1;
    int srcReg2;
    int imm5;

    destReg = CURRENT_LATCHES.REGS[(currInstruction&0x0E00)>>9];
    srcReg1 = CURRENT_LATCHES.REGS[(currInstruction&0x01c0)>>6];
    srcReg2 = CURRENT_LATCHES.REGS[(currInstruction&0x0007)];

    imm5 = Low16bits(currInstruction&0x001f);
    sext(&imm5,4);
    if (!(currInstruction & 0x0020)){
        NEXT_LATCHES.REGS[(currInstruction&0x0E00)>>9] = Low16bits(srcReg1 + srcReg2);
    }
    else{
        NEXT_LATCHES.REGS[(currInstruction&0x0E00)>>9] = Low16bits(srcReg1 + imm5);
	// still need to set condition codes
    }
    if(isPositive(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9])){NEXT_LATCHES.P = 1;}
    if(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9]==0){NEXT_LATCHES.Z = 1;}
    if(isNegative(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9])){NEXT_LATCHES.N = 1;}
    // requires testing
}
void br(int currInstruction){
    //if ((n AND N) OR (z AND Z) OR (p AND P))
    //PC = PCü + LSHF(SEXT(PCoffset9), 1);
    int offset9 = currInstruction & 0x01ff;
    sext(&offset9,8);
    if(CURRENT_LATCHES.N || CURRENT_LATCHES.P || CURRENT_LATCHES.Z){
      NEXT_LATCHES.PC = CURRENT_LATCHES.PC + (offset9 << 1);   
    }
    // requires testing
}
void jmp(int currInstruction){
    
    //
    //PC = BaseR
    NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[(currInstruction&0x01c0)>>6]);
    // requires testing
}
void jsrr(int currInstruction){
	//TEMP = PC†
	//	if (bit(11)==0)
	//	   PC = BaseR;
	//	else
	//	   PC = PC† + LSHF(SEXT(PCoffset11), 1);
	//      R7 = TEMP;
	//			    		    		
	//* PC†: incremented PC
    NEXT_LATCHES.REGS[7] = Low16bits(NEXT_LATCHES.PC);
    int offset11 = currInstruction&0x07ff;
    sext(&offset11,10);
    if(!(currInstruction&0x0800)){
      NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[(currInstruction&0x01c0)>>6];	
    }else{
         NEXT_LATCHES.PC = NEXT_LATCHES.PC + (offset11<<1);
     }    	
    // requires testing
}
void ldb(int currInstruction){
    //DR = SEXT(mem[BaseR + SEXT(boffset6)]);
    //setcc();
    int baseR = (currInstruction&0x01c0)>>6;
    int imm6 = currInstruction&0x003f;
    sext(&imm6,5);
    NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9] = Low16bits(MEMORY[(CURRENT_LATCHES.REGS[baseR]+imm6)>>1][0]);
    if(isPositive(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9])){NEXT_LATCHES.P = 1;}
    if(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9]==0){NEXT_LATCHES.Z = 1;}
    if(isNegative(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9])){NEXT_LATCHES.N = 1;}
    // requires testing
}
void ldw(int currInstruction){
    //DR = MEM[BaseR + LSHF(SEXT(offset6), 1)];
    //setcc();
    int baseR = (currInstruction&0x01c0)>>6;
    int imm6 = currInstruction&0x003f;
    sext(&imm6,5);
    int destination;
    destination  = MEMORY[(CURRENT_LATCHES.REGS[baseR]+imm6)>>1][0];
    destination  = destination + (MEMORY[(CURRENT_LATCHES.REGS[baseR]+imm6)>>1][1]<<8);
    NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9] = Low16bits(destination);
    if(isPositive(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9])){NEXT_LATCHES.P = 1;}
    if(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9]==0){NEXT_LATCHES.Z = 1;}
    if(isNegative(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9])){NEXT_LATCHES.N = 1;}
    // requires testing

}
void lea(int currInstruction){
    //DR = PCë + LSHF(SEXT(PCoffset9),1);
    int offset9 = currInstruction&0x01ff;
    sext(&offset9,8);
      NEXT_LATCHES.REGS[(currInstruction & 0x0e00)>>9] = Low16bits(CURRENT_LATCHES.PC + (offset9 << 1));   
}
void not(int currInstruction){
    //if (bit[5] == 0)
    //DR = SR1 XOR SR2;
    //else
    //DR = SR1 XOR SEXT(imm5);
    //setcc();

    int destR = (currInstruction&0x0e00)>>9;
    int sourceR1 = (currInstruction&0x01c0)>>6;
    int sourceR2 = currInstruction&0x0007;
    int imm5 = currInstruction&0x001f;
    sext(&imm5,4);
    if (!currInstruction&0x0020){
       NEXT_LATCHES.REGS[destR] = Low16bits(NEXT_LATCHES.REGS[sourceR1] ^ NEXT_LATCHES.REGS[sourceR2]);
    }
    else{
       NEXT_LATCHES.REGS[destR] = Low16bits(NEXT_LATCHES.REGS[sourceR1] ^ imm5);
    }
    if(isPositive(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9])){NEXT_LATCHES.P = 1;}
    if(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9]==0){NEXT_LATCHES.Z = 1;}
    if(isNegative(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9])){NEXT_LATCHES.N = 1;}
    // requires testing
    
}
void shf(int currInstruction){
    //if (bit[4] == 0)
    //DR = LSHF(SR, amount4);
    //else
    //  if (bit[5] == 0)
    //    DR = RSHF(SR, amount4, 0);
    //  else
    //   DR = RSHF(SR, amount4, SR[15]);
    //setcc();
      int destR = (currInstruction&0x0e00)>>9;
      int sourceR = (currInstruction&0x01c0)>>6;
    if(!(currInstruction&0x0010)){
      NEXT_LATCHES.REGS[destR] = Low16bits(CURRENT_LATCHES.REGS[sourceR]<<(currInstruction&0x000f));
    }else 
      if(!currInstruction&0x0020){
            unsigned int value = (unsigned int)CURRENT_LATCHES.REGS[sourceR];
            value = value >> (currInstruction&0x000f);
	    NEXT_LATCHES.REGS[destR] = Low16bits((int) value);
          }else {
	    int result = CURRENT_LATCHES.REGS[sourceR] >> (currInstruction&0x000f);
	    sext(&result,6);
            NEXT_LATCHES.REGS[destR] = Low16bits(result); 
 	   }
    if(isPositive(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9])){NEXT_LATCHES.P = 1;}
    if(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9]==0){NEXT_LATCHES.Z = 1;}
    if(isNegative(NEXT_LATCHES.REGS[(currInstruction&0x0e00)>>9])){NEXT_LATCHES.N = 1;}
    // requires testing
}
void stb(int currInstruction){
    //mem[BaseR + SEXT(boffset6)] = SR[7:0];
    int baseR = (currInstruction&0x01c0)>>6;
    int offset6 = currInstruction&0x003f;
    sext(&offset6,5);
    int sourceR = (currInstruction&0x0e00)>>9;
    MEMORY[(CURRENT_LATCHES.REGS[baseR]+offset6)>>1][0] = Low16bits(CURRENT_LATCHES.REGS[sourceR] & 0x00ff);
    // requires testing
}
void stw(int currInstruction){
    //MEM[BaseR + LSHF(SEXT(offset6), 1)] = SR;
    int baseR = (currInstruction&0x01c0)>>6;
    int offset6 = currInstruction&0x003f;
    
    sext(&offset6,5);

    int sourceR = (currInstruction&0x0e00)>>9;
    int right8 = CURRENT_LATCHES.REGS[sourceR]&0x00ff;
    int left8 = (CURRENT_LATCHES.REGS[sourceR]&0xff00)>>8;
    MEMORY[(CURRENT_LATCHES.REGS[baseR]+ (offset6<<1))>>1][0] = Low16bits(right8);
    MEMORY[(CURRENT_LATCHES.REGS[baseR]+ (offset6<<1))>>1][1] = Low16bits(left8);
}
void trap(int currInstruction){
    //R7 = PCë ;
    //PC = MEM[LSHF(ZEXT(trapvect8), 1)];
    NEXT_LATCHES.REGS[7] = Low16bits(NEXT_LATCHES.PC);
    NEXT_LATCHES.PC = 0;
    // requires testing
}
void sext(int *number, int bit){
    int num = *number;
    num = num >> bit;
    num = num & 0x0001;
    if(num){
      if(bit ==4){
        *number = *number | 0xffffffe0;
      } 
      if(bit ==8){ 
        *number = *number | 0xfffffe00;
      } 
      if(bit ==10){
        *number = *number | 0xfffff800;
      } 
      if(bit ==5){
        *number = *number | 0xffffffc0;
      }
      if(bit == 6) {
	*number = *number | 0xffffff80;
      }
    }
}
int isNegative(int num){
    if(num&0x0080){return TRUE;}
    else return FALSE;
}
int isPositive(int num){
    if(num&0x0080){return FALSE;}
    if(num == 0){return FALSE;}
    else return TRUE;
}
