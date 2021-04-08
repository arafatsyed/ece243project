//FUNCTION HEADERS
//*********************************
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
void set_A9_IRQ_stack ();
void config_GIC ();
void config_interval_timer ();
void config_KEYs ();
void enable_A9_interrupts ();
void disable_A9_interrupts();
void config_PS2();
void keyboard_ISR();
void config_interrupt(int N, int CPU_target);
void init_game();
void eraseVisual(int count);
void updateVisual();
bool drawVisual();
void delay(int delayT);
void callbackVisual(double velocityInitial, double theta);

struct Basketball{
	int dy,dx, x, y, prevX, prevY, startX,startY;
};
struct Net{
	int x, y, prevX, prevY;
	bool score;
};

struct Player{
	int x, y, prevX,prevY, playerID;
	
};

struct PowerBar{
	int xSlider,ySlider, prevXSlider, prevYSlider, width, height, xFixed, yFixed;
	int powerArray[10];
};

struct AimBar{
	int xEnd,yEnd, prevXEnd, prevYEnd, xFixed, yFixed;
	int rightX,rightY;
	int topX, topY;
	double angle;
};
struct Game{
	struct Basketball basketball;
	struct Net net;
	struct Player player;
	struct PowerBar powerBar;
	struct AimBar aimBar;
	uint16_t background[240][320];
	int gameState;
};
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

#define GAMESTATE_INTRO 0
#define GAMESTATE_CHARACTER 1
#define GAMESTATE_INSTRUCTION 2
#define GAMESTATE_ANGLE 3
#define GAMESTATE_POWER 4
#define GAMESTATE_TIMING 5
#define GAMESTATE_VISUAL 6
#define GAMESTATE_SCORE 7
#define GAMESTATE_DIFFICULTY 8
#define GAMESTATE_END -1
#define BALL_SPAWN_X 37
#define BALL_SPAWN_Y 140
#define BALL_DIAMETER 15
#define DELAY 100000
// Begin main.c 

 // global variable
volatile int pixel_buffer_start;
struct Game game;

//backgrounds to use
const uint16_t testscreen[240][320] ={0};
const uint16_t basketballModel[15][15] = {
	{51168,51168,51168,51168,51168,0,0,0,0,0,51168,51168,51168,51168,51168},
	{51168,51168,51168,0,0,64164,64164,0,64164,64164,0,0,51168,51168,51168},
	{51168,51168,0,64164,64164,64164,64164,0,64164,64164,64164,64164,0,51168,51168},
	{51168,0,0,64164,64164,64164,64164,0,64164,64164,64164,64164,0,0,51168},
	{51168,0,64164,0,64164,64164,64164,0,64164,64164,64164,0,64164,0,51168},
	{0,64164,64164,64164,0,64164,64164,0,64164,64164,0,64164,64164,0,0},
	{0,64164,64164,64164,64164,0,64164,0,64164,0,64164,64164,64164,64164,0},
	{0,0,64164,64164,64164,64164,0,0,0,64164,64164,64164,64164,0,0},
	{0,64164,0,0,0,0,0,0,0,0,0,0,0,64164,0},
	{0,64164,64164,64164,64164,0,64164,0,64164,0,64164,64164,64164,64164,0},
	{51168,0,64164,64164,0,64164,64164,0,64164,64164,0,64164,64164,0,51168},
	{51168,0,64164,0,64164,64164,64164,0,64164,64164,64164,0,64164,0,51168},
	{51168,51168,0,64164,64164,64164,64164,0,64164,64164,64164,64164,0,51168,51168},
	{51168,51168,51168,0,0,64164,64164,0,64164,0,0,0,51168,51168,51168},
	{51168,51168,51168,51168,51168,0,0,0,0,0,51168,51168,51168,51168,51168},

};

int main(void)
{	
	init_game();
	set_A9_IRQ_stack();
	
	config_GIC();
	
	config_PS2();
	
	enable_A9_interrupts();
	
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    // declare other variables(not shown)
    // initialize location and direction of rectangles(not shown)
	
   	
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
	
	callbackVisual(10.5,60);
}
void callbackVisual(double velocityInitial, double theta){
	double velocity = velocityInitial;
	double angle = theta;
	
	angle = angle*3.1415/180.0;
	
    game.basketball.dy = (int) -1*velocity*sin(angle);
	game.basketball.dx = (int) velocity*cos(angle);
	int count =0;
	while (1)
    {	
	
			//erase
		eraseVisual(count);
			
		//update directions
		updateVisual();
		
		
		delay(DELAY);
		
		
		//update positions
		if(!drawVisual()){
			break;
		}
			
		
		count++;
	}
}
bool drawVisual(){
	game.basketball.prevX= game.basketball.x;
	game.basketball.prevY=game.basketball.y;
	game.basketball.x+=game.basketball.dx;
	game.basketball.y+=game.basketball.dy;
	if(game.basketball.dy == 0 && game.basketball.dx ==0 && game.basketball.y +BALL_DIAMETER>=RESOLUTION_Y ){
		game.basketball.y = game.basketball.startY;
		game.basketball.x = game.basketball.startX;
		game.gameState=GAMESTATE_SCORE;
		return FALSE;
	}	
	// code for drawing the boxes and lines (not shown)
	
	for(int y=game.basketball.y;y<game.basketball.y+BALL_DIAMETER;y++){

		for(int x=game.basketball.x;x<game.basketball.x+BALL_DIAMETER;x++){

			if(basketballModel[y-game.basketball.y][x-game.basketball.x] != 51168){
				if(x>=0 && x<=RESOLUTION_X && y >= 0 && y<=RESOLUTION_Y){
					plot_pixel(x,y,basketballModel[y-game.basketball.y][x-game.basketball.x]);
				}
			}

		}

	}
	// code for updating the locations of boxes (not shown)
	
	
	wait_for_vsync(); // swap front and back buffers on VGA vertical sync
	pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
	return TRUE;
}
void delay(int delayT){
	int f = delayT;
	while(f !=0){
		f--;
	}
}
void updateVisual(){
	if (game.basketball.x >= RESOLUTION_X-BALL_DIAMETER)
		game.basketball.dx= -1* abs(game.basketball.dx/2);
	if (game.basketball.x <= 0)
		game.basketball.dx= abs(game.basketball.dx/2);
	if(game.basketball.y >= RESOLUTION_Y-BALL_DIAMETER){
		game.basketball.dy = -1*abs(game.basketball.dy/2);
		game.basketball.dx = 0;
	}
	if(game.basketball.y +BALL_DIAMETER<RESOLUTION_Y){
		game.basketball.dy+=1;
	}
}
void eraseVisual(int count){
	if(count !=0){
		for(int y=game.basketball.prevY;y<game.basketball.prevY+BALL_DIAMETER;y++){

			for(int x=game.basketball.prevX;x<game.basketball.prevX+BALL_DIAMETER;x++){

				if(x>=0 && x<=RESOLUTION_X && y >= 0 && y<=RESOLUTION_Y){
					plot_pixel(x,y,BLACK);
				}

			}

		}
	}
}	

void init_game(){
	game.gameState = 0;
	
	game.basketball.x=BALL_SPAWN_X;game.basketball.y= BALL_SPAWN_Y; game.basketball.dy =0;game.basketball.dx=0; game.basketball.prevX=0; game.basketball.prevY=0;
	game.basketball.startX = BALL_SPAWN_X; game.basketball.startY=BALL_SPAWN_Y;
	
	game.net.x=0;game.net.y=0;game.net.prevX=0; game.net.prevY=0; game.net.score=false;
	
	game.player.x=0; game.player.y=0;game.player.prevX=0;game.player.prevY=0;game.player.playerID=0;
	
	game.powerBar.xSlider=0;game.powerBar.ySlider=0; game.powerBar.prevXSlider=0; game.powerBar.prevYSlider=0; game.powerBar.width=0; 
	game.powerBar.height=0;game.powerBar.xFixed=0;game.powerBar.yFixed=0;
	
	for (int i = 0; i<10 ; i++){
		game.powerBar.powerArray[i] = (i+1)*5;
	}
	
	game.aimBar.xEnd=0;game.aimBar.yEnd=0;game.aimBar.prevXEnd=0; game.aimBar.prevYEnd=0; game.aimBar.xFixed=0; game.aimBar.yFixed=0;game.aimBar.rightX=0;
	game.aimBar.rightY=0; game.aimBar.topX=0; game.aimBar.topY=0;game.aimBar.angle=0.0;
	
	for (int x=0 ; x < RESOLUTION_X; x++)
	{
		for (int y=0 ; y < RESOLUTION_Y; y++)
		{
			game.background[x][y] = testscreen[x][y];
		}
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
	
	
	