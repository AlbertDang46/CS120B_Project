/*
 * CS120B_Project.c
 *
 * Created: 11/30/2019 7:07:17 PM
 * Author : Albert Dang adang018
 */ 

#include <avr/io.h>
#include <stdlib.h>
#include "timer.h"
#include "scheduler.h"
#include "io.h"
#include "eeprom.h"
#include "ledmatrix.h"
#include "stack.h"

#define BTN_UP (~PIND & 0x01)
#define BTN_RIGHT (~PIND & 0x02)
#define BTN_DOWN (~PIND & 0x04)
#define BTN_LEFT (~PIND & 0x08)
#define NUM_SYMBOLS 8
#define SYMBOL_HEIGHT 8


enum snake_directions { UP, RIGHT, DOWN, LEFT };
typedef struct SnakeGame {
    led_pos* snake_head;
	unsigned char snake_length;
	Stack pos_stack;
	unsigned char dir;
	led_pos fruit_pos;
	unsigned char collided;
	unsigned char** led_bitmap;
	unsigned char led_cur_row;
	unsigned char score;
} SnakeGame;

SnakeGame snakeGame;

const unsigned char SYMBOL_LIST[NUM_SYMBOLS][SYMBOL_HEIGHT] = {
	{ 0x11, 0x0A, 0x04, 0x0A, 0x15, 0x0A, 0x04, 0x00 },
	{ 0x11, 0x0A, 0x04, 0x0A, 0x15, 0x0A, 0x04, 0x00 },
	{ 0x11, 0x0A, 0x04, 0x0A, 0x15, 0x0A, 0x04, 0x00 },
	{ 0x11, 0x0A, 0x04, 0x0A, 0x15, 0x0A, 0x04, 0x00 },
	{ 0x11, 0x0A, 0x04, 0x0A, 0x15, 0x0A, 0x04, 0x00 },
	{ 0x11, 0x0A, 0x04, 0x0A, 0x15, 0x0A, 0x04, 0x00 },
	{ 0x11, 0x0A, 0x04, 0x0A, 0x15, 0x0A, 0x04, 0x00 },
	{ 0x11, 0x0A, 0x04, 0x0A, 0x15, 0x0A, 0x04, 0x00 },
};

void SnakeGameInit(SnakeGame* game) {
	game->snake_length = 1;
	game->pos_stack = StackInit(65535);
	
	led_pos temp;
	temp.x = 4;
	temp.y = 4;
	StackPush(game->pos_stack, temp);
	game->snake_head = StackTopPointer(game->pos_stack);
	
	game->dir = LEFT;
	
	game->fruit_pos.x = 3;
	game->fruit_pos.y = 4;
	
	game->collided = 0;
	
	snakeGame.led_bitmap = (unsigned char**)malloc(MATRIX_HEIGHT * sizeof(unsigned char*));
	unsigned char i;
	for(i = 0; i < MATRIX_HEIGHT; i++) {
		snakeGame.led_bitmap[i] = (unsigned char*)malloc(MATRIX_WIDTH * sizeof(unsigned char));
	}
	clearMatrix(game->led_bitmap, WHITE);
	
	game->led_cur_row = 0;
	game->score = 0;
}

void updateSnakePos(SnakeGame* game) {
	led_pos temp;
	if(game->dir == UP) {
		temp.x = game->snake_head->x;
		temp.y = game->snake_head->y - 1;
	} else if(game->dir == RIGHT) {
		temp.x = game->snake_head->x + 1;
		temp.y = game->snake_head->y;
	} else if(game->dir == DOWN) {
		temp.x = game->snake_head->x;
		temp.y = game->snake_head->y + 1;
	} else if(game->dir == LEFT) {
		temp.x = game->snake_head->x - 1;
		temp.y = game->snake_head->y;
	}	
	StackPush(game->pos_stack, temp);
	game->snake_head = StackTopPointer(game->pos_stack);
}

void checkCollision(SnakeGame* game) {
	if(game->snake_head->x < 0 || game->snake_head->x >= MATRIX_WIDTH || game->snake_head->y < 0 || game->snake_head->y >= MATRIX_HEIGHT) {
		game->collided = 1;
	}
	
	led_pos* temp;
	unsigned char i;
	for(i = 1; i < game->snake_length; i++) {
		temp = StackTopPointer(game->pos_stack) - i;
		if(game->snake_head->x == temp->x && game->snake_head->y == temp->y) {
			game->collided = 1;
		}
	}
}

void checkEatFruit(SnakeGame* game) {
	if(game->snake_head->x == game->fruit_pos.x && game->snake_head->y == game->fruit_pos.y) {
		game->snake_length++;
		game->fruit_pos.x = rand() % 8;
		game->fruit_pos.y = rand() % 8;
		game->score++;
	}
}

void displayEnd(SnakeGame* game) {
	clearMatrix(game->led_bitmap, BLACK);
}

void updateGame(SnakeGame* game) {
	if(game->collided) {
		clearMatrix(game->led_bitmap, WHITE);
		return;
	}
	
	clearMatrix(game->led_bitmap, WHITE);
	unsigned char i;
	for(i = 0; i < game->snake_length; i++) {
		setMatrixDotPos(game->led_bitmap, *(game->snake_head - i), BLACK);
	}
	setMatrixDotPos(game->led_bitmap, game->fruit_pos, BLACK);
}

void updateHighScores(SnakeGame* game) {
	EEPROM_write(0, EEPROM_read(1));
	EEPROM_write(1, game->score);
}

void initializeLCD() {
	LCD_init();
	unsigned char i;
	for(i = 0; i < NUM_SYMBOLS; i++) {
		LCD_LoadSymbol(i, SYMBOL_LIST[i]);
	}
}

void displayScoreLCD(SnakeGame* game) {
	LCD_DisplaySymbol(1, 0);
	LCD_Cursor(3);
	LCD_WriteData(game->score + '0');
	LCD_DisplaySymbol(5, 0);
}

void displayHighScoresLCD() {
	LCD_ClearScreen();
	
	LCD_DisplayString(1, "Last Score");
	LCD_DisplaySymbol(11, 0);
	LCD_Cursor(13);
	LCD_WriteData((EEPROM_read(0) / 10) + '0');
	LCD_WriteData((EEPROM_read(0) % 10) + '0');
	
	LCD_DisplayString(17, "This Score");
	LCD_DisplaySymbol(11, 0);
	LCD_Cursor(13);
	LCD_WriteData((EEPROM_read(1) / 10) + '0');
	LCD_WriteData((EEPROM_read(1) % 10) + '0');
}

enum snake_States { snake_start, snake_init, snake_play, snake_end };
int snakeSMTick(int state) {
	switch(state) {
		case snake_start:
			state = snake_init;
			break;
		case snake_init:
			state = snake_play;
			break;
		case snake_play:
			state = snakeGame.collided ? snake_end : snake_play;
			break;
		case snake_end:
			state = snake_end;
			break;
		default:
			state = snake_start;
			break;
	}	
	switch(state) {
		case snake_start:
			break;
		case snake_init:
			SnakeGameInit(&snakeGame);
			updateGame(&snakeGame);
			break;
		case snake_play:
			updateSnakePos(&snakeGame);
			checkCollision(&snakeGame);
			checkEatFruit(&snakeGame);
			updateGame(&snakeGame);
			break;
		case snake_end:
			displayEnd(&snakeGame);
			updateHighScores(&snakeGame);
			break;
		default:
			break;
	}
	
	return state;
};

enum direction_States { direction_start, direction_turn };
int directionSMTick(int state) {
	switch(state) {
		case direction_start:
			state = direction_turn;
			break;
		case direction_turn:
			state = direction_turn;
			break;
		default:
			state = direction_start;
			break;
	}
	switch(state) {
		case direction_start:
			break;
		case direction_turn:
			if(BTN_UP && !BTN_RIGHT && !BTN_DOWN && !BTN_LEFT && snakeGame.dir != DOWN) {
				snakeGame.dir = UP;
			} else if(!BTN_UP && BTN_RIGHT && !BTN_DOWN && !BTN_LEFT && snakeGame.dir != LEFT) {
				snakeGame.dir = RIGHT;
			} else if(!BTN_UP && !BTN_RIGHT && BTN_DOWN && !BTN_LEFT && snakeGame.dir != UP) {
				snakeGame.dir = DOWN;
			} else if(!BTN_UP && !BTN_RIGHT && !BTN_DOWN && BTN_LEFT && snakeGame.dir != RIGHT) {
				snakeGame.dir = LEFT;
			}
			break;
		default:
			break;
	}
	
	return state;
};

enum ledmp_States { ledmp_start, ledmp_display };
int ledmpSMTick(int state) {
	switch(state) {
		case ledmp_start:
			state = ledmp_display;
			break;
		case ledmp_display:
			state = ledmp_display;
			break;
		default:
			state = ledmp_start;
			break;
	}
	switch(state) {
		case ledmp_start:
			break;
		case ledmp_display:
			LED_ROWOUT = LED_ROWS[snakeGame.led_cur_row];
			LED_COLOUT = getLedCols(snakeGame.led_bitmap, snakeGame.led_cur_row);
			snakeGame.led_cur_row = (snakeGame.led_cur_row == (MATRIX_HEIGHT - 1)) ? 0 : snakeGame.led_cur_row + 1;
			break;
		default:
			break;
	}
	
	return state;
}

enum lcd_States { lcd_start, lcd_init, lcd_score, lcd_highscores };
int lcdSMTick(int state) {
	switch(state) {
		case lcd_start:
			state = lcd_init;
			break;
		case lcd_init:
			state = lcd_score;
			break;
		case lcd_score:
			state = snakeGame.collided ? lcd_highscores : lcd_score;
			break;
		case lcd_highscores:
			state = lcd_highscores;
			break;
		default:
			break;
	}
	switch(state) {
		case lcd_start:
			break;
		case lcd_init:
			initializeLCD();
			break;	
		case lcd_score:
			displayScoreLCD(&snakeGame);
			break;
		case lcd_highscores:
			displayHighScoresLCD();
			break;
		default:
			break;
	}
	
	return state;
};

int main(void)
{
    DDRA = 0xFF; PORTA = 0x00;
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xC0; PORTD = 0x3F;

    static task task1, task2, task3, task4;
    task* tasks[] = { &task1, &task2, &task3, &task4 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    task1.state = snake_start;
    task1.period = 50;
    task1.elapsedTime = task1.period;
    task1.TickFct = &snakeSMTick;
	
	task2.state = direction_start;
	task2.period = 1;
	task2.elapsedTime = task2.period;
	task2.TickFct = &directionSMTick;
	
	task3.state = ledmp_start;
	task3.period = 1;
	task3.elapsedTime = task3.period;
	task3.TickFct = &ledmpSMTick;
	
	task4.state = lcd_start;
	task4.period = 50;
	task4.elapsedTime = task4.period;
	task4.TickFct = &lcdSMTick;

    unsigned long GCD = tasks[0]->period;
    unsigned short n;
    for(n = 1; n < numTasks; n++) {
	    GCD = findGCD(GCD, tasks[n]->period);
    }
		
    TimerSet(GCD);
    TimerOn();

    unsigned short i;
    while (1) {
	    for(i = 0; i < numTasks; i++) {
		    if(tasks[i]->elapsedTime == tasks[i]->period) {
			    tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			    tasks[i]->elapsedTime = 0;
		    }
		    tasks[i]->elapsedTime += GCD;
	    }
	    while(!TimerFlag);
	    TimerFlag = 0;
    }
    return 0;
}

