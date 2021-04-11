//FUNCTION HEADERS
//*********************************
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
	
void set_A9_IRQ_stack ();
void config_GIC ();
void config_interval_timer ();
void config_KEYs ();
void enable_A9_interrupts ();
void disable_A9_interrupts();
void config_PS2();
void keyboard_ISR();
void config_interrupt(int N, int CPU_target);

//****************************************
//END FUNCTION  HEADERS 

int main(void){
	
	
	set_A9_IRQ_stack();
	
	config_GIC();
	
	config_PS2();
	
	enable_A9_interrupts();
	
	while(1){
		
	}
	
	
	
}

//enabling interrupts
void enable_A9_interrupts(void)
{
	int status = 0b01010011;
	asm("msr cpsr, %[ps]" : : [ps]"r"(status));
	
}

void disable_A9_interrupts(void) {
	int status = 0b11010011;
	asm("msr cpsr, %[ps]" : : [ps] "r"(status));
}
// Define the remaining exception handlers */
void __attribute__ ((interrupt)) __cs3_isr_undef (void)
{
	while (1);
}
void __attribute__ ((interrupt)) __cs3_isr_swi (void)
{
	while (1);
}
void __attribute__ ((interrupt)) __cs3_isr_pabort (void)
{
	while (1);
}
void __attribute__ ((interrupt)) __cs3_isr_dabort (void)
{
	while (1);
}
void __attribute__ ((interrupt)) __cs3_isr_fiq (void)
{
	while (1);
}


/* Define the IRQ exception handler */
void __attribute__ ((interrupt)) __cs3_isr_irq (void)
{
	// Read the ICCIAR from the processor interface
	int int_ID = *((int *) 0xFFFEC10C);

	if (int_ID == 79){
		
		keyboard_ISR();
	}
	else{
		while (1){} // if unexpected, then stay here
	}
	// Write to the End of Interrupt Register (ICCEOIR)
	*((int *) 0xFFFEC110) = int_ID;
	return;
}

void config_GIC()
{
	config_interrupt(79,1);

	// Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all priorities
	*((int *) 0xFFFEC104) = 0xFFFF;
	// Set CPU Interface Control Register (ICCICR). Enable signaling of interrupts
	*((int *) 0xFFFEC100) = 1; // enable = 1
	// Configure the Distributor Control Register (ICDDCR) to send pending interrupts to CPUs
	*((int *) 0xFFFED000) = 1; // enable = 1
	
	return;
}

void set_A9_IRQ_stack()
{
	int stack, mode;
	stack = 0xFFFFFFFF - 7; // top of A9 on-chip memory, aligned to 8 bytes
	/* change processor to IRQ mode with interrupts disabled */
	mode = 0b11010010;
	asm("msr cpsr, %[ps]" : : [ps] "r" (mode));
	/* set banked stack pointer */
	asm("mov sp, %[ps]" : : [ps] "r" (stack));
	/* go back to SVC mode before executing subroutine return! */
	mode = 0b11010011;
	asm("msr cpsr, %[ps]" : : [ps] "r" (mode));
}

void config_interrupt(int N, int CPU_target) {
	int reg_offset, index, value, address;
	/* Configure the Interrupt Set-Enable Registers (ICDISERn).
	* reg_offset = (integer_div(N / 32) * 4
	* value = 1 << (N mod 32) */
	reg_offset = (N >> 3) & 0xFFFFFFFC;
	index = N & 0x1F;
	value = 0x1 << index;
	address = 0xFFFED100 + reg_offset;
	/* Now that we know the register address and value, set the appropriate bit */
	*(int *)address |= value;
	/* Configure the Interrupt Processor Targets Register (ICDIPTRn)
	* reg_offset = integer_div(N / 4) * 4
	* index = N mod 4 */
	reg_offset = (N & 0xFFFFFFFC);
	index = N & 0x3;
	address = 0xFFFED800 + reg_offset + index;
	/* Now that we know the register address and value, write to (only) the
	* appropriate byte */
	*(char *)address = (char)CPU_target;
}

void config_PS2(){
	
	printf("config_psr");
	volatile int* PS2_Data = (int*)0xFF200100; //PS2 DATA register
	*(PS2_Data +1) = 0x00000001; //writes 1 into PS2_Control register to enable interrupts;	
	
}

void keyboard_ISR(){
	
	
	volatile int* LED = (int*)0xff200000;
	
	volatile int* PS2_data_reg = (int*)0xff200100; //PS/2 Data register
	
	int PS2_data = *(PS2_data_reg); //gets data from register
	
	int RVALID; 
	
	RVALID = (PS2_data & 0x8000); //getting RVALID field
	
	unsigned char keyPressed = (PS2_data & 0xff); //gets last byte, holds the key that was pressed
	
	int interruptCode = *(PS2_data_reg+1); //gets interrupt code
	
	*(PS2_data_reg + 1) = interruptCode;	//clears the interrupt
	
	
	if(RVALID!=0){
		
		if(keyPressed == 0x1c){ //pressed A
			
			*LED = 1;
			
		}
		else{
		
			*LED = 0x0;
		}		
	}

	
	return;
}
