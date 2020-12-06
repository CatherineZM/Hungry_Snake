#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "image.h"
#include "objects.h"
#include "interruptID.h"
#include "definition.h"
#include "address_map_arm.h"

volatile int *KEY_EDGE_ptr = (int *) 0xFF20005C;
volatile int *SW_ptr = (int *) 0xFF200040;
volatile int *pixel_ctrl_ptr = (int *) 0xFF203020;
volatile int pixel_buffer_start;

int headtype = 0;//0 for down, 1 for up, 2 for right, 3 for left
int score = 0;
char click = 1;
char preClicked = 1;
int speed;
bool foodGet = false;
bool gameOver = false;
bool gameStart = false; //switch 0 to start the game
bool AImode = false; //switch 1 to start AI mode
bool gameRestart = false;
bool drawNewFruit = false; 
snake *head;
fruit newFruit;
snakeHead newHead;
Body newBody;

char up = 'w';
char down = 's';
char left = 'a';
char right = 'd';

char pixels[24][24];

void creatBoard(void);
void creatFood(void);
void MovingBody(void);
void ChangeBody(int x,int y);
void Eating(void);
int Judge(void);
//exceptions setup
void set_A9_IRQ_stack(void);
void config_GIC(void);

void config_HPS_GPIO1(void);
void config_KEYs(void);
void enable_A9_interrupts(void);

void pushbutton_ISR(void);

//draw functions setup
void plot_pixel(int x, int y, short int line_color){
    *(short *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void plot_line(int x0, int y0, int x1, int y1, short int line_color);

void plot_image(int x0, int y0, int imageArrary[], unsigned width, unsigned height);

void clear_screen(void);

bool wait_for_vsync(void);

void plot_body(Body *myBody);

void plot_head(snakeHead *myHead);

void plot_fruits(fruit *myFruit);

void plot_background(void);

void clear_stuffs(void);

//get the first element of the snake linklist
snake snakeFront(snake *Head);
//get the last element of the snake linklist
snake snakeBack(snake *Head);
//insert new element at front and return the new head, you should use it in the form of Head = snakePush_Front(Head)
snake* snakePush_Front(snake *Head, int x, int y);
//insert new element at back and return the new head, you should use it in the form of Head = snakePush_Back(Head, ...)
snake* snakePush_Back(snake *Head, int x, int y);
//delete the first element and return new head, you should use it in the form of Head = snakePop_Front(Head, ...)
snake* snakePop_Front(snake *Head);
//delete the last element and return new head, you should use it in the form of Head = snakePop_Back(Head)
snake* snakePop_Back(snake *Head);
//delete all the elements in the list, you should use it in the form of Head = snakeClear(Head)
snake* snakeClear(snake *Head);
//get the size of the  list
int snakeSize(snake *Head);
//list(Head) = list(newHead), you should use it in the form of Head = snakeCopy(Head, newHead)
snake* snakeCopy(snake *Head, snake *newHead);
//return a null pointer(an empty list)
snake* snakeNew(void);

int main(void){
    set_A9_IRQ_stack();
    config_GIC();
    config_KEYs(); // configure pushbutton KEYs to generate interrupts
    enable_A9_interrupts(); // enable interrupts
    
    *(pixel_ctrl_ptr + 1) = 0xC8000000;//store address in the back buffer
    wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    
    unsigned SW_value;
    
    newGame: {
        gameOver = false;
        gameStart = true;
		drawNewFruit = true; 
        score = 0;
		creatBoard();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }
    *KEY_EDGE_ptr = 0xF;
    while (1)
    {
        clear_screen();
		 
        SW_value = (unsigned int) *SW_ptr;
        if(SW_value == 0b0000000001){
            gameStart = true;
        }else if(SW_value == 0b0000000010){
            AImode = true;
        }else if(SW_value == 0b0000000100){
			gameRestart = true;
		}else{
			gameStart = false; 
			gameRestart = false; 
			AImode = false; 
		}
        
        if(!gameStart){
            goto nextFrame;
        }
		if(gameRestart){
			goto newGame;
		}
		if(gameOver){
            goto gameover;
        }
        plot_background();
		pushbutton_ISR();
        MovingBody();
		creatFood();
        Eating();
        
        gameover:
        {
            if(gameOver){
                plot_image(0, 0, image_gameover_320x240, 320, 240);
            
                SW_value = (unsigned int) *SW_ptr;
                if(SW_value == 0b0000000100){
                    gameStart = true;
                    gameOver = false;
                    goto newGame;
                }
                wait_for_vsync();
                pixel_buffer_start = *(pixel_ctrl_ptr + 1);
                goto gameover;
            }
        }
        
        nextFrame:{
            wait_for_vsync();
            pixel_buffer_start = *(pixel_ctrl_ptr + 1);
        }
        if(!gameStart){
            goto newGame;
        }
    }
    return 0;
}

//initialized the game board
void creatBoard(void) {
    int i;
    int j;

    //srand((int)time(NULL));

    head = (snake*)malloc(sizeof(snake));
    snake *p = (snake*)malloc(sizeof(snake));
    snake *q = (snake*)malloc(sizeof(snake));

    head->x = rand() % (240) + 15;
    head->y = rand() % (220) + 10;

    p->x = head->x;
    p->y = head->y + 24;
    q->x = head->x;
    q->y = head->y + 48;

    head->next = p;
    p->next = q;
    q->next = NULL;

}

void creatFood(void) {
    int emptyCount = 57600 - snakeSize(head)*576;
    int count = 0;
    int fruit_x, fruit_y;
    int type = rand()%2;
    srand((int)time(NULL));
    int randC = rand() % emptyCount;
    int i, j;
	if(drawNewFruit){
		
		for(i = 0; i < 240; i ++) {
			for(j = 0; j < 240; j ++) {
				if(pixels[i][j] == 0 || pixels[i][j] == 2) {
					if(count == randC) {
						fruit_x = i + 40;
						fruit_y = j;
					}
					count ++;
				}
			}
		}
		newFruit = (fruit){.x = fruit_x, .y = fruit_y, .image_W = 24,.image_H = 24, .fruit_type = type};
		drawNewFruit = false;
	}
	plot_fruits(&newFruit);
}

void MovingBody(void) {
    int count = 0;
    int x = head->x, y = head->y;
    snake *p = head;
    
    while (1)
    {
        if (p == NULL) break;
        pixels[p->x][p->y] = emptyPixel;
        count++;
        p = p->next;
    }
    if(click == up){
        if(preClicked != down){
            y -= 24;
			headtype = 0;
        }else {
            click = down;
            y += 24;
			headtype = 1;
        }
        
    }else if(click == down){
		if(preClicked != up) {
            y += 24;
			headtype = 1;
        }else {
            click = up;
            y -= 24;
			headtype = 0;
        }
        
    }else if(click == left){
        if(preClicked != right){
            x -= 24;
			headtype = 3;
        }else {
            click = right;
            x += 24;
			headtype = 2;
        }
        
    }else if(click == right){
        if(preClicked != left){
            x += 24;
			headtype = 2;
        }else {
            click = left;
            x -= 24;
			headtype = 3;
        }
        
    }
        
    if(x != head->x || y != head->y) {
        ChangeBody(x, y);
    }
    p = head->next;
    newHead = (snakeHead) {.x = head->x, .y = head->y, .image_W = 24, .image_H = 24, .head_type = headtype};
    plot_head(&newHead);

    if(Judge() == 0) {
        gameOver = true;
    }else {
        gameOver = false;
    }

    while (1)
    {
        if (p == NULL){
            break;
        }
        newBody = (Body){.x = p->x, .y = p->y, .image_W = 24, .image_H = 24};
        plot_body(&newBody);
        p = p->next;
    }

    // if (count <= 10) speed = 150;
    // else if (count > 10 && count <= 20) speed = 100;
    // else if (count > 20 && count <= 40) speed = 50;
    // else speed = 10;
    speed = 10;
    wait_for_vsync();
}

void ChangeBody(int x,int y)
{
    snake *p = head;
    while (p->next->next != NULL) {
        p = p->next;
    }
    
    if(!foodGet) {
        
        free(p->next);
        p->next = NULL;
    
    }
    
    foodGet = false;
    
    snake *new_head = (snake*)malloc(sizeof(snake));
    new_head->x = x;
    new_head->y = y;
    new_head->next = head;
    head = new_head;
}

void Eating(void)
{
    if (head->x == newFruit.x&&head->y == newFruit.y)
    {
        snake *_new = (snake*)malloc(sizeof(snake));
        snake *p;
        p = head;
        while (1)
        {
            if (p->next == NULL) break;
            p = p->next;
        }
        score += 10;
        foodGet = true;
		drawNewFruit = true;
    }
}

int Judge(void)
{

    if (head->x < 39|| head->x > 279 || head->y < 0 || head->y > 239)
    {
        return 0;
    }
    snake *p = head->next;
    while (1)
    {
        if (p->next == NULL){
            break;
        }
        if (head->x == p->x && head->y == p->y)
        {
            return 0;
        }
        p = p->next;
    }
    return 1;
}

snake snakeFront(snake *Head) {
    return *Head;
}
snake snakeBack(snake *Head) {
    snake *p = Head;
    while (1)
    {
        if (p->next == NULL){
            break;
        }
        p = p->next;
    }
    return *p;
}

snake* snakePush_Front(snake *Head, int x, int y) {
    snake *p = (snake*)malloc(sizeof(snake));
    p->x = x;
    p->y = y;
    p->next = Head;
    return p;
}

snake* snakePush_Back(snake *Head, int x, int y) {
    if(snakeSize(Head) == 0) {
        snake *q = (snake*)malloc(sizeof(snake));
        q->x = x;
        q->y = y;
        q->next = NULL;
        return q;
    }
    snake *p = Head;
    while (1)
    {
        if (p->next == NULL){
            break;
        }
        p = p->next;
    }
    snake *q = (snake*)malloc(sizeof(snake));
    q->x = x;
    q->y = y;
    q->next = NULL;
    p->next = q;
    return Head;
}

snake* snakePop_Front(snake *Head) {
    if(snakeSize(Head) == 0){
        return NULL;
    }
    if(snakeSize(Head) == 1) {
        free(Head);
        return NULL;
    }
    snake *p = Head->next;
    free(Head);
    return p;
}

snake* snakePop_Back(snake *Head) {
    if(snakeSize(Head) == 0){
         return NULL;
    }
    if(snakeSize(Head) == 1) {
        free(Head);
        return NULL;
    }
    snake *p = Head;
    while (1)
    {
        if (p->next->next == NULL){
            break;
        }
        p = p->next;
    }
    free(p->next);
    p->next = NULL;
    return Head;
}

snake* snakeClear(snake *Head) {
    snake *p = Head, *q;
    while (p != NULL) {
        q = p->next;
        free(p);
        p = q;
    }
    return NULL;
}

int snakeSize(snake *Head) {
    snake *p = Head;
    int count = 0;
    if (Head == NULL){
        return 0;
    }
    while (1)
    {
        count ++;
        if (p->next == NULL){
            break;
        }
        p = p->next;
    }

    return count;
}

snake* snakeCopy(snake *Head, snake *newHead) {
    Head = snakeClear(Head);
    snake *p = newHead;
    while(1) {
        if(p == NULL) {
            break;
        }
        snake q;
        q.x = p->x;
        q.y = p->y;
        q.next = p->next;
        Head = snakePush_Back(Head, q.x, q.y);
        p = p->next;
    }
    return Head;
}

snake* snakeNew(void) {
    return NULL;
}
   
void swap(int *left, int *right){
    int temp = *left;
    *left = *right;
    *right = temp;
}

void plot_line(int x0, int y0, int x1, int y1, short int line_color){
    bool is_steep = (abs(y1 - y0) > abs(x1 - x0));
    if(is_steep){
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if(x0 > x1){
        swap(&x0, &x1);
        swap(&y0, &y1);
    }
    
    int delta_x = (x1 - x0);
    int delta_y = (abs(y1 - y0));
    int error = (-(delta_x / 2));
    int y = y0;
    int y_step;
    if(y0 < y1){
        y_step = 1;
    }else{
        y_step = -1;
    }
    int x;
    for(x = x0; x <= x1; x++){
        if(is_steep){
            plot_pixel(y, x, line_color);
        }else{
            plot_pixel(x, y, line_color);
        }
        error = delta_y + error;
        if(error >= 0){
            y = y + y_step;
            error = error - delta_x;
        }
    }
}

void plot_image(int initial_x, int initial_y, int imageArray[], unsigned width, unsigned height){
    int i = 0;
    unsigned y, x;
    
    for(y = 0; y < height; y++){
        for(x = 0; x < width; x++){
            int plot_x = initial_x + x;
            int plot_y = initial_y + y;
            
            if (imageArray[i] != (int)0b1111100000011111 && plot_x >= 0 && plot_y >= 0 && plot_x < 320 && plot_y < 240){
                plot_pixel(plot_x, plot_y, imageArray[i]);
            }
            
            i++;
        }
    }
}

void clear_screen(void){
    int x,y;
    for(y = 0; y < 240; y++){
        for(x = 0; x < 320; x++){
            plot_pixel(x, y, 0xFFFF);
        }
    }
}

bool wait_for_vsync(void){
    register int status;;
    
    *pixel_ctrl_ptr = 1;
    
    status = *(pixel_ctrl_ptr + 3);
    while((status  & 0x01) != 0){
        status = *(pixel_ctrl_ptr + 3);
    }
    
    return true;
}

void plot_body(Body *myBody){
    plot_image(myBody->x, myBody->y, image_body_24x24, myBody->image_W, myBody->image_H);
}

void plot_head(snakeHead *myHead){
    plot_image(myHead->x, myHead->y, headSelection[myHead->head_type], myHead->image_W, myHead->image_H);
}

void plot_fruits(fruit *myFruit){
    plot_image(myFruit->x, myFruit->y, fruitSelection[rand() % 2], myFruit->image_W, myFruit->image_H);
}
void plot_background(void){
	plot_line(37, 0, 37, 239, 0x8000);
	plot_line(38, 0, 38, 239, 0x8000);
	plot_line(39, 0, 39, 239, 0x8000);
	plot_line(279, 0, 279, 239, 0x8000);
	plot_line(280, 0, 280, 239, 0x8000);
	plot_line(281, 0, 281, 239, 0x8000);
	
}
void clear_stuffs(){
    unsigned x, y;
    for (y = 0; y < 239; y++){
        for(x = 40; x < 279; x++){
            plot_pixel(x, y, 0xFFFF);
        }
    }
}

/* setup the KEY interrupts in the FPGA */
void config_KEYs(void) {
    volatile int * KEY_ptr = (int *) KEY_BASE; // pushbutton KEY address
    *(KEY_ptr + 2) = 0xF; // enable interrupts for all KEYs
}

/*
 * Initialize the banked stack pointer register for IRQ mode
 */
void set_A9_IRQ_stack(void) {
    int stack, mode;
    stack = A9_ONCHIP_END - 7; // top of A9 onchip memory, aligned to 8 bytes
    /* change processor to IRQ mode with interrupts disabled */
    mode = INT_DISABLE | IRQ_MODE;
    asm("msr cpsr, %[ps]" : : [ps] "r"(mode));
    /* set banked stack pointer */
    asm("mov sp, %[ps]" : : [ps] "r"(stack));
    /* go back to SVC mode before executing subroutine return! */
    mode = INT_DISABLE | SVC_MODE;
    asm("msr cpsr, %[ps]" : : [ps] "r"(mode));
}

/*
 * Turn on interrupts in the ARM processor
 */
void enable_A9_interrupts(void) {
    int status = SVC_MODE | INT_ENABLE;
    asm("msr cpsr, %[ps]" : : [ps] "r"(status));
}

/*
 * Configure the Generic Interrupt Controller (GIC)
 */
void config_GIC(void) {
    int address; // used to calculate register addresses
    /* configure the HPS timer interrupt */
    *((int *) 0xFFFED8C4) = 0x01000000;
    *((int *) 0xFFFED118) = 0x00000080;
    /* configure the FPGA interval timer and KEYs interrupts */
    *((int *) 0xFFFED848) = 0x00000101;
    *((int *) 0xFFFED108) = 0x00000300;
    // Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all
    // priorities
    address = MPCORE_GIC_CPUIF + ICCPMR;
    *((int *) address) = 0xFFFF;
    // Set CPU Interface Control Register (ICCICR). Enable signaling of
    // interrupts
    address = MPCORE_GIC_CPUIF + ICCICR;
    *((int *) address) = ENABLE;
    // Configure the Distributor Control Register (ICDDCR) to send pending
    // interrupts to CPUs
    address = MPCORE_GIC_DIST + ICDDCR;
    *((int *) address) = ENABLE;
}


// Define the IRQ exception handler

void __attribute__ ((interrupt)) __cs3_isr_irq(void) {
    // Read the ICCIAR from the processor interface
    int address = MPCORE_GIC_CPUIF + ICCIAR;
    int int_ID = *((int *) address);
    if (int_ID == KEYS_IRQ){ // check if interrupt is from the KEYs
        pushbutton_ISR();
    }else{
        while (1); // if unexpected, then stay here
    }
    // Write to the End of Interrupt Register (ICCEOIR)
    address = MPCORE_GIC_CPUIF + ICCEOIR;
    *((int *) address) = int_ID;
    return;
}

// Define the remaining exception handlers

void __attribute__ ((interrupt)) __cs3_reset(void) {
    while (1)
        ;
}

void __attribute__ ((interrupt)) __cs3_isr_undef(void) {
    while (1)
        ;
}

void __attribute__ ((interrupt)) __cs3_isr_swi(void) {
    while (1)
        ;
}

void __attribute__ ((interrupt)) __cs3_isr_pabort(void) {
    while (1)
        ;
}

void __attribute__ ((interrupt)) __cs3_isr_dabort(void) {
    while (1)
        ;
}

void __attribute__ ((interrupt)) __cs3_isr_fiq(void) {
    while (1)
        ;
}

void pushbutton_ISR(void) {
    volatile int * KEY_ptr = (int *) 0xFF200050;
    int press;
    press = *(KEY_ptr + 3); // read the pushbutton interrupt register
    *(KEY_ptr + 3) = press; // Clear the interrupt
    if (press == 0b0001) { //press KEY[0]
        click = up;
    } else if (press == 0b0010) { // press KEY[1]
        click = down;
    } else if (press == 0b0100){ //press KEY[2]
        click = right;
    }else if (press = 0b1000){ //press KEY[3]
        click = left;
    }
    return;
}
