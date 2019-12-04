#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#include <string.h>

#define LED_ROWOUT PORTA
#define LED_COLOUT PORTB
#define MATRIX_HEIGHT 8
#define MATRIX_WIDTH 8
#define BLACK 1
#define WHITE 0


typedef struct led_pos {
	unsigned char x;
	unsigned char y;	
} led_pos;

char LED_ROWS[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
char LED_COLS[] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F };
	
void clearMatrix(unsigned char** bitmap, unsigned char color);
void setMatrixDotXY(unsigned char** bitmap, unsigned char x, unsigned char y, unsigned char color);
void setMatrixDotPos(unsigned char** bitmap, led_pos pos, unsigned char color);
void setMatrixBitmap(unsigned char** bitmap, unsigned char** new_bitmap);
unsigned char getLedCols(unsigned char** bitmap, unsigned char row);

void clearMatrix(unsigned char** bitmap, unsigned char color) {
	unsigned char i, j;
	for(i = 0; i < MATRIX_HEIGHT; i++) {
		for(j = 0; j < MATRIX_WIDTH; j++) {
			bitmap[i][j] = color;
		}
	}
}

void setMatrixDotXY(unsigned char** bitmap, unsigned char x, unsigned char y, unsigned char color) {
	bitmap[x][y] = color;
}

void setMatrixDotPos(unsigned char** bitmap, led_pos pos, unsigned char color) {
	setMatrixDotXY(bitmap, pos.x, pos.y, color);
}

void setMatrixBitmap(unsigned char** bitmap, unsigned char** new_bitmap) {
	unsigned char i, j;
	for(i = 0; i < MATRIX_HEIGHT; i++) {
		for(j = 0; j < MATRIX_WIDTH; j++) {
			bitmap[i][j] = new_bitmap[i][j];
		}
	}
}

unsigned char getLedCols(unsigned char** bitmap, unsigned char row) {
	unsigned char col_out = 0xFF;
	unsigned char i;
	for(i = 0; i < MATRIX_WIDTH; i++) {
		unsigned char col_value = bitmap[row][i] ? LED_COLS[i] : 0xFF;
		col_out &= col_value;
	}
	return col_out;
}

#endif //LEDMATRIX_H