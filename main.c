#include "main.h"
/*
 int main(int argc, char *argv[])
 {
 if(argc != 2)
 {
 printf("Usage: %s QS\n", argv[0]);
 exit(1);
 }
 QS = (int)(*argv[1] - '0');
 printf("%d", QS);
 openFile();
 partition();
 return 0;
 }
 */

int main() {
	QS = 2;
	openFile();
	partition();
	return 0;
}

int openFile() {
	int i, j;
	unsigned char buff;
	unsigned char line[MAXLINE];

	pFile = fopen(pFileName, "rb");

	if (NULL == pFile) {
		perror("Error Opening File");
		return -1;
	}

	/*
	 * Remove Header
	 */
	for (i = 0; i < HEAD; i++) {
		getc(pFile);
	}

	for (i = 0; i < HEIGHT; i++) {
		for (j = 0; j < WIDTH; j++) {
			buff = (unsigned char) fgetc(pFile);
			imgArray[i][j] = (double) buff - 128;
		}
	}

	if (ferror(pFile)) {
		perror("Error Reading File");
		fclose(pFile);
		return -1;
	}

	fclose(pFile);

	pFile = fopen("jpeg.book", "rt");
	if (NULL == pFile) {
		perror("Error Opening File");
		return -1;
	}

	for (i = 0; i < DCLINE; i++) {
		fscanf(pFile, "%s", line);
		dcCode[i].category = line[0];
		fscanf(pFile, "%s", dcCode[i].code);
	}

	for (i = 0; i < ACLINE; i++) {
		fscanf(pFile, "%s", line);
		acCode[i].run = line[0];
		fscanf(pFile, "%s", line);
		acCode[i].level = line[0];
		fscanf(pFile, "%s", acCode[i].code);

		printf("%c  %c  %s\n", acCode[i].run, acCode[i].level, acCode[i].code);
	}

	fclose(pFile);

	return 0;
}

void partition() {
	int i, j, m, n;
	double (*p)[8];
	double* ptr[8];

	fp = fopen("encoded.txt", "wt");
	pFile = fopen("output.jpg", "wb");
	if (NULL == pFile) {
		perror("Error Creating File");
		return;
	}
	/*
	for (i = 0; i < sizeof(head); i++) {
		fputc(head[i], pFile);
	}
	*/

	ptblk = 0;
	for (i = 0; i < VBLK; i++) {
		pblk = 0;
		for (j = 0; j < HBLK; j++) {
			for (m = 0; m < 8; m++) {
				for (n = 0; n < 8; n++) {
					imgBlock[m][n] = imgArray[i * 8 + m][j * 8 + n];
				}
			}
			p = imgBlock;
			for (m = 0; m < 8; m++) {
				ptr[m] = p[m];
			}
			ddct8x8s(-1, ptr);

			for(m = 0; m < 8; m++)
			{
				for(n = 0; n < 8; n++)
				{
					printf("%f ", imgBlock[m][n]);
				}
			}
			printf("\n");
			quantize();
			zigzag();
			dpcmRle();
			pblk++;
			ptblk++;
		}
	}
	fclose(pFile);
}

void quantize() {
	int j, k;
	double t;
	for (j = 0; j < 8; j++) {
		for (k = 0; k < 8; k++) {
			t = imgBlock[j][k];
			imgBlock[j][k] = (nearbyint)(t / quanMatrix[j][k] / QS);
		}
	}
}

void zigzag() {
	int i, j = 0, k = 0;
	DCTcoeff[0] = imgBlock[j][k];
	for (i = 1; i <= 63; i++) {
		switch (zigzagIndex[i - 1]) {
		case 0:
			k++;
			break;
		case 1:
			j++;
			k--;
			break;
		case 2:
			j++;
			break;
		case 3:
			j--;
			k++;
			break;
		default:
			break;
		}
		DCTcoeff[i] = imgBlock[j][k];
	}
	for (i = 0; i < 64; i++) {
		printf("%d ", (int) DCTcoeff[i]);
	}
	printf("\n");
}

void dpcmRle() {
	int dc, dcCategory;
	char firstElem[200], secondElem[15];
	unsigned char temp;
	int tRun = 0, tLevel;
	int i = 1, j, running = 0;

	/*
	 * DC
	 */
	if (pblk == 0) {
		dc = (int) DCTcoeff[0];
	} else {
		dc = (int) DCTcoeff[0] - preDC;
	}

	if (dc == 0) {
		strcpy(firstElem, dcCode[dc].code);
		secondElem[0] = '0';
		secondElem[1] = 0;
	} else {
		dcCategory = logb(abs(dc)) + 1;
		strcpy(firstElem, dcCode[dcCategory].code);
		strcpy(secondElem, genCode(dc, dcCategory));
	}
	strcat(firstElem, secondElem);

	/*
	 * AC
	 */
	do {
		if (DCTcoeff[i] == 0 && running == 0) {
			running = 1;
			tRun++;
		} else if (DCTcoeff[i] == 0 && running == 1) {
			if (tRun == 15) {
				tLevel = 0;
				strcat(firstElem, acCode[F0].code);
				tRun = 0;
			} else if (i == 63) {
				strcat(firstElem, "1010");
			} else {
				tRun++;
			}
		} else if (DCTcoeff[i] != 0 && running == 1) {
			running = 0;
			tLevel = logb(abs(DCTcoeff[i])) + 1;
			for (j = 0; j < ACLINE; j++) {
				if ((tRun == acCode[j].run - '0' || tRun == acCode[j].run - 'A')
						&& (tLevel == acCode[j].level - '0'
								|| tLevel == acCode[j].level - 'A')) {
					strcat(firstElem, acCode[j].code);
					break;
				}
			}
			strcpy(secondElem, genCode(DCTcoeff[i], tLevel));
			strcat(firstElem, secondElem);
			tRun = 0;
		} else if (DCTcoeff[i] != 0 && running == 0) {
			tRun = 0;
			tLevel = logb(abs(DCTcoeff[i])) + 1;
			for (j = 0; j < ACLINE; j++) {
				if ((tRun == acCode[j].run - '0' || tRun == acCode[j].run - 'A')
						&& (tLevel == acCode[j].level - '0'
								|| tLevel == acCode[j].level - 'A')) {
					strcat(firstElem, acCode[j].code);
					break;
				}
			}
			strcpy(secondElem, genCode(DCTcoeff[i], tLevel));
			strcat(firstElem, secondElem);
		}
		i++;
	} while (i <= 63);

	// Writing into file
	j = 0;
	while (firstElem[j] != 0) {
		if (elemPtr < 8) {
			elem[elemPtr] = firstElem[j];
			j++;
			elemPtr++;
		} else if (elemPtr == 8) {
			temp = 0;
			for (i = 0; i < 8; i++) {
				if (elem[i] == '1') {
					temp |= mask[i];
				}
			}
			fputc(temp, pFile);
			if (temp == 0xFF){
			// 0x00 follows 0xFF
				fputc(0, pFile);
			}
			elemPtr = 0;
		}
	}

	if (ptblk == TBLK-1) {
		while (elemPtr < 8) {
			elem[elemPtr] = 0;
			elemPtr++;
		}
		for (i = 0; i < 8; i++) {
			if (elem[i] == '1') {
				temp |= mask[i];
			}
		}
		fputc(temp, pFile);
		/*
		fputc(EOI[0], pFile);
		fputc(EOI[1], pFile);
		*/
		printf("finish");
		printf("here\n");
	}
	printf("%s\n\n", firstElem);
	preDC = (int) DCTcoeff[0];
}

char* genCode(int dc, int dcCategory) {
	char code[dcCategory + 1];
	int diff, i = dcCategory - 1;
	if (dc < 0) {
		diff = dc + (int) pow(2.0, (double) dcCategory) - 1;
	} else {
		diff = dc;
	}

	do {
		code[i] = '0' + diff % 2;
		i--;
		diff /= 2;
	} while (i >= 0);
	code[dcCategory] = 0;
	return code;
}
