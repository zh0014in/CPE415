#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "dct.h"

int main();
int openFile();
void partition();
void quantize();
void zigzag();
void dpcmRle();
char* genCode(int, int);
void ddct8x8s(int i, double **a);

#define FILENAME "barbara.pgm"
#define HEIGHT 512
#define WIDTH 512
#define HEAD 57
#define HBLK (WIDTH/8)
#define VBLK (HEIGHT/8)
#define TBLK HBLK*VBLK
#define MAXLINE 23
#define DCLINE 12
#define ACLINE 162
#define F0 151

#define DEBUG_MODE 0

typedef unsigned char byte;
struct dcCodeBook {
	char category;
	char code[10];
};
struct acCodeBook {
	char run;
	char level;
	char code[17];
};

const char* pPGMName = FILENAME;
FILE* pPGM;
FILE* pBOOK;
FILE* pOUT;
FILE* pBYTEOUT;
double imgArray[WIDTH][HEIGHT];
double imgBlock[8][8];
const int quanMatrix[8][8] = { { 16, 11, 10, 16, 24, 40, 51, 61 }, { 12, 12, 14,
		19, 26, 58, 60, 55 }, { 14, 13, 16, 24, 40, 57, 69, 56 }, { 14, 17, 22,
		29, 51, 87, 80, 62 }, { 18, 22, 37, 56, 68, 109, 103, 77 }, { 24, 35,
		55, 64, 81, 104, 113, 92 }, { 49, 64, 78, 87, 103, 121, 120, 101 }, {
		72, 92, 95, 98, 112, 100, 103, 99 } };
double DCTcoeff[64];
const int zigzagIndex[] = { 0, 1, 2, 3, 3, 0, 1, 1, 1, 2, 3, 3, 3, 3, 0, 1, 1,
		1, 1, 1, 2, 3, 3, 3, 3, 3, 3, 0, 1, 1, 1, 1, 1, 1, 1, 0, 3, 3, 3, 3, 3,
		3, 2, 1, 1, 1, 1, 1, 0, 3, 3, 3, 3, 2, 1, 1, 1, 0, 3, 3, 2, 1, 0 };

double testBlock[8][8] = { { 100, 100, 74, 73, 82, 81, 54, 56 }, { 60, 59, 56,
		57, 50, 49, 53, 53 }, { 50, 52, 58, 47, 52, 48, 47, 51 }, { 48, 47, 47,
		48, 44, 47, 50, 70 }, { 61, 67, 47, 60, 39, 40, 87, 86 }, { 92, 90, 57,
		93, 48, 43, 112, 56 }, { 93, 121, 67, 84, 86, 91, 72, 66 }, { 92, 114,
		113, 62, 54, 58, 77, 160 } };
int QS;
struct acCodeBook acCode[ACLINE];
struct dcCodeBook dcCode[DCLINE];
int preDC;
int pblk, ptblk;
static char mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
int elemPtr = 0;
unsigned char elem[8];
#endif
