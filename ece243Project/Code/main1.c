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
/* This files provides address values that exist in the system */

#define SDRAM_BASE            0xC0000000
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_CHAR_BASE        0xC9000000

/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define TIMER_BASE            0xFF202000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030
//sdsdf
/* VGA colors */
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00
#define BLACK 0x0000
	
#define ABS(x) (((x) > 0) ? (x) : -(x))

/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240

/* Constants for animation */
#define BOX_LEN 2
#define NUM_BOXES 1

#define FALSE 0
#define TRUE 1

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// Begin part3.c code for Lab 7


volatile int pixel_buffer_start; // global variable

int main(void)
{
	set_A9_IRQ_stack();
	
	config_GIC();
	
	config_PS2();
	
	enable_A9_interrupts();
	
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    // declare other variables(not shown)
    // initialize location and direction of rectangles(not shown)
	
	int x_box[NUM_BOXES];
	int y_box[NUM_BOXES];
	int dx_box[NUM_BOXES];
	int dy_box[NUM_BOXES];
	int colour_box[NUM_BOXES];
	
				//printf ("Decimals: %d \n", dx_box[0]);
	dy_box[0] = -15;
	dx_box[0] = 10;
	y_box[0] = RESOLUTION_Y - 80;
	x_box[0] = 20;
   	
	/* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
	clear_screen();
	
	int count =0;
	while (1)
    {	
					//printf ("old: %d \n", x_box[0]);
        /* Erase any boxes and lines that were drawn in the last iteration */
		if(count !=0)
			for(int i =0; i<NUM_BOXES; i++){
				plot_box(x_box[i]-dx_box[i],y_box[i]-dy_box[i],BLACK);
				//draw_line(x_box[i]-dx_box[i],y_box[i]-dy_box[i], x_box[(i+1)%NUM_BOXES]-dx_box[(i+1)%NUM_BOXES],y_box[(i+1)%NUM_BOXES]-dy_box[(i+1)%NUM_BOXES], BLACK);
				//printf ("Decimals: %d \n", (i+1)%NUM_BOXES);
			}
		//update directions
		for(int i =0; i<NUM_BOXES; i++){
			if (x_box[i] >= RESOLUTION_X-2)
				dx_box[i]=-1;
			if (x_box[i] <= 0)
				dx_box[i]=1;
			if(y_box[i] >= RESOLUTION_Y-2)
				dy_box[i] = -1;
			if (y_box[i] <= 0)
				dy_box[i] =1;
		}
		int f = 50000*2;
		while(f !=0){
			f--;
		}
		dy_box[0] = dy_box[0] + 1;
		//update positions
		for(int i =0; i<NUM_BOXES; i++){
			x_box[i] = x_box[i] +dx_box[i];
			y_box[i] = y_box[i] +dy_box[i];
		}
			//printf ("Updated: %d ", x_box[0]);
        // code for drawing the boxes and lines (not shown)
		for(int i =0; i<NUM_BOXES; i++){
			plot_box(x_box[i],y_box[i],RED);
			//draw_line(x_box[i],y_box[i], x_box[(i+1)%NUM_BOXES],y_box[(i+1)%NUM_BOXES], colour_box[i]);
		}
        // code for updating the locations of boxes (not shown)
		
		
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    	count++;
	}
}
void plot_box(int x, int y, short int line_color){
	plot_pixel(x,y,line_color);
	plot_pixel(x+1,y,line_color);
	plot_pixel(x,y+1,line_color);
	plot_pixel(x+1,y+1,line_color);
}
void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void clear_screen(){
	for (int x=0; x< RESOLUTION_X; x++){
		for (int y = 0 ; y<RESOLUTION_Y; y++){
			plot_pixel(x,y,BLACK);
		}
	}
}

void draw_line(int x0, int y0,int x1, int y1, short int line_color){
	bool is_steep = (ABS(y1-y0)>ABS(x1-x0));
	
	if(is_steep){
		int temp1 = x0;
		x0 = y0;
		y0 = temp1;
		temp1 = x1;
		x1= y1;
		y1 = temp1;
	}if(x0>x1){
		int temp1 = x0;
		x0 = x1;
		x1= temp1;
		temp1 = y0;
		y0=y1;
		y1 = temp1;
	}
	int deltax = x1-x0;
	int deltay = abs(y1-y0);
	int error = -(deltax/2);
	int y = y0;
	int y_step;
	y_step =(y0<y1)?1:-1;
	
	for(int x = x0; x<=x1 ;x++){
		
		if(is_steep){
			plot_pixel(y,x,line_color);
		}
		else{
			plot_pixel (x,y,line_color);
		}
		error =error+deltay;
		if(error >=0){ 
			y = y+y_step;
			error = error-deltax;
		}
		
	}
}

void wait_for_vsync(){
	volatile int* pixel_ctrl_ptr= (int*) 0xFF203020;
	int status;
	*pixel_ctrl_ptr =1;
	status = *(pixel_ctrl_ptr+3);
	while((status & 0x01)!=0){
		status = *(pixel_ctrl_ptr+3);
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
	