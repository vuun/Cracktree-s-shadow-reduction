#define	_CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "lighting_percolation.h"
#include "pmd.h"

#define I_no 4
#if I_no == 1
int	IMG_WIDTH = (392);
int	IMG_HEIGHT = (190);
#define filename "dark_shadow_392x190.pgm"
#elif I_no == 2
int	IMG_WIDTH = (320);
int	IMG_HEIGHT = (220);
#define filename "dlight_shadow_320x220.pgm"

#elif I_no == 3
int	IMG_WIDTH = (320);
int	IMG_HEIGHT = (220);
#define filename "dark_shadow_320x220.pgm"

#elif I_no == 4
int	IMG_WIDTH = (659);
int	IMG_HEIGHT = (494);
#define filename "No.01.pgm"
#endif

/************************/
/*	ShadowRemoval本体	*/
/************************/

/************************/
/*	8ビット画像読み込み	*/
/************************/
void read_pgm_file(LIGHTING_UINT8* cp, LIGHTING_UINT32 width, LIGHTING_UINT32 height, LIGHTING_INT8* fname)
{
	char head[100];
	char *c;
	int w1, h1, d;									/*	これら使ってない(というかヘッダ部分を取得するためのダミー?)	*/

	FILE *fp = fopen((const char*)fname, "rb");

	do
	{
		fgets(head, 256, fp);						/*	行の最初の文字が数字でない限りheadに繰り返し読み込み	*/
	} while (head[0]<'0' || head[0]>'9');

	w1 = strtol(head, &c, 10);						/*	720	*/
	h1 = strtol(c, NULL, 10);						/*	480(スペース付き)	*/

	fgets(head, 256, fp);

	d = strtol(head, &c, 10);						/*	255	*/

	if (fp)
	{
		fread(cp, sizeof(char), width*height, fp);	/*	fpから1バイト分のデータをwidth*height個分取得しcpに格納?	*/
		fclose(fp);
	}
	else
	{
		printf("can't open file in write_pgm_file(%s).\n", fname);
	}
}

/************************/
/*	8ビット画像書き込み	*/
/************************/
void write_pgm_file(LIGHTING_UINT8* cp, LIGHTING_UINT32 width, LIGHTING_UINT32 height, LIGHTING_INT8* fname)
{
	FILE *fp = fopen(fname, "wb");

	if (fp)
	{
		fprintf(fp, "P5\n%d %d\n255\n", width, height);
		fwrite(cp, sizeof(char), width*height, fp);
		fclose(fp);
	}

	else
	{
		printf("can't open file in write_pgm_file(%s).\n", fname);
	}
}

/****************************************/
/*	8ビット画像書き込みテストプログラム	*/
/****************************************/
void main() {
	LIGHTING_UINT8* readImg = NULL;
	LIGHTING_UINT8*	mmCloseImg = NULL;
	LIGHTING_UINT8* gauSmoothImg = NULL;
	LIGHTING_UINT8*	geoLevelImg = NULL;
	LIGHTING_UINT8*	PMDImg = NULL;
	LIGHTING_ERROR_t retVal = LIGHTING_OK;
	LIGHTING_INT8 frname[64], fwname[64];
	time_t Time;
	struct tm *Now;

	/*	メモリ(画素数分)の確保	*/
	readImg = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)*IMG_WIDTH*IMG_HEIGHT);
	mmCloseImg = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)*IMG_WIDTH*IMG_HEIGHT);
	gauSmoothImg = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)*IMG_WIDTH*IMG_HEIGHT);
	geoLevelImg = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)*IMG_WIDTH*IMG_HEIGHT);
	PMDImg = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)*IMG_WIDTH*IMG_HEIGHT);
	/*	メモリ(画素数分)の初期化	*/
	memset(readImg, 0, sizeof(unsigned char)*IMG_WIDTH*IMG_HEIGHT);
	memset(mmCloseImg, 0, sizeof(unsigned char)*IMG_WIDTH*IMG_HEIGHT);
	memset(gauSmoothImg, 0, sizeof(unsigned char)*IMG_WIDTH*IMG_HEIGHT);
	memset(geoLevelImg, 0, sizeof(unsigned char)*IMG_WIDTH*IMG_HEIGHT);

	sprintf(frname, filename);
	read_pgm_file(readImg, IMG_WIDTH, IMG_HEIGHT, frname);
#if 0
	sprintf(frname, "No.03_3_gauss10.pgm");
	read_pgm_file(gauSmoothImg, IMG_WIDTH, IMG_HEIGHT, frname);
#else	
	retVal = mmClose(readImg, mmCloseImg);
	time(&Time);
	Now = localtime(&Time);
	sprintf(fwname, "output\\%4d_%2d_%2d_%2d_%2d_%2d_01mmClose_%s", 1900 + Now->tm_year, Now->tm_mon + 1, Now->tm_mday, Now->tm_hour, Now->tm_min, Now->tm_sec, filename);
	write_pgm_file(mmCloseImg, IMG_WIDTH, IMG_HEIGHT, fwname);

	retVal = gauSmooth(mmCloseImg, gauSmoothImg);
	time(&Time);
	Now = localtime(&Time);
	sprintf(fwname, "output\\%4d_%2d_%2d_%2d_%2d_%2d_02gauSmooth_%s", 1900 + Now->tm_year, Now->tm_mon + 1, Now->tm_mday, Now->tm_hour, Now->tm_min, Now->tm_sec, filename);
	write_pgm_file(gauSmoothImg, IMG_WIDTH, IMG_HEIGHT, fwname);

#endif
	retVal = geoLevel(readImg, gauSmoothImg, geoLevelImg);

	free(readImg);
	free(mmCloseImg);
	free(gauSmoothImg);
	free(geoLevelImg);

	return;
}

/********************/
/*	Step1.mmClose	*/
/********************/
LIGHTING_ERROR_t mmClose(LIGHTING_UINT8* pInImg, LIGHTING_UINT8* pOutImg) {
	LIGHTING_ERROR_t retVal = LIGHTING_OK;
	LIGHTING_INT32 x, y, dx, dy, max = 255, min = 0;
	LIGHTING_UINT8* rTmpImg = LIGHTING_NULL;
	LIGHTING_UINT8* cTmpImg = LIGHTING_NULL;

	LIGHTING_INT32 xmin = MAX_VALUE8, xmax = MIN_VALUE, ymin, ymax, times, max_times = 2;
	double a, b;

#define MASK_SIZE 7
#if MASK_SIZE==7
	LIGHTING_INT32 mask[MASK_SIZE][MASK_SIZE] = {
		{ 0, 0, 0, 1, 0, 0, 0 },
		{ 0, 0, 1, 1, 1, 0, 0 },
		{ 0, 1, 1, 1, 1, 1, 0 },
		{ 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 1, 1, 1, 1, 1, 0 },
		{ 0, 0, 1, 1, 1, 0, 0 },
		{ 0, 0, 0, 1, 0, 0, 0 }
	};
#elif MASK_SIZE==5
	LIGHTING_INT32 mask[MASK_SIZE][MASK_SIZE] = {
		{ 0, 0, 1, 0, 0 },
		{ 0, 1, 1, 1, 0 },
		{ 1, 1, 1, 1, 1 },
		{ 0, 1, 1, 1, 0 },
		{ 0, 0, 1, 0, 0 }
	};
#elif MASK_SIZE==3
	LIGHTING_INT32 mask[MASK_SIZE][MASK_SIZE] = {
		{ 0, 1, 0 },
		{ 1, 1, 1 },
		{ 0, 1, 0 }
	};

#endif

	/*	メモリ確保	*/
	rTmpImg = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)*IMG_WIDTH*IMG_HEIGHT);
	cTmpImg = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)*IMG_WIDTH*IMG_HEIGHT);
	/*	メモリ初期化	*/
	memset(rTmpImg, 0, sizeof(unsigned char)*IMG_WIDTH*IMG_HEIGHT);
	memset(cTmpImg, 0, sizeof(unsigned char)*IMG_WIDTH*IMG_HEIGHT);
	memcpy(rTmpImg, pInImg, sizeof(unsigned char)*IMG_WIDTH*IMG_HEIGHT);

	/*	膨張	*/
	for (times = 0; times<max_times; times++) {
		for (y = 0; y<IMG_HEIGHT; y++) {
			for (x = 0; x<IMG_WIDTH; x++) {
				max = 0;
				for (dy = 0; dy<MASK_SIZE; dy++) {			/* 構造要素高さ。*/
					for (dx = 0; dx<MASK_SIZE; dx++) {		/* 構造要素幅。*/
						if (mask[dy][dx] == 1) {
							if ((y - MASK_SIZE / 2 + dy) >= 0 && (y - MASK_SIZE / 2 + dy)<IMG_HEIGHT && (x - MASK_SIZE / 2 + dx) >= 0 && (x - MASK_SIZE / 2 + dx)<IMG_WIDTH) {
								if (rTmpImg[(y - MASK_SIZE / 2 + dy)*IMG_WIDTH + (x - MASK_SIZE / 2 + dx)]>max) {
									max = rTmpImg[(y - MASK_SIZE / 2 + dy)*IMG_WIDTH + (x - MASK_SIZE / 2 + dx)];
								}
							}
						}
					}
				}
				cTmpImg[y*IMG_WIDTH + x] = (LIGHTING_UINT8)max;
			}
		}
		memcpy(rTmpImg, cTmpImg, sizeof(unsigned char)*IMG_WIDTH*IMG_HEIGHT);
	}

	/*	収縮	*/
	for (times = 0; times<max_times; times++) {
		for (y = 0; y<IMG_HEIGHT; y++) {
			for (x = 0; x<IMG_WIDTH; x++) {
				min = 255;
				for (dy = 0; dy<MASK_SIZE; dy++) {			/* 構造要素高さ。*/
					for (dx = 0; dx<MASK_SIZE; dx++) {		/* 構造要素幅。*/
						if (mask[dy][dx] == 1) {
							if ((y - MASK_SIZE / 2 + dy) >= 0 && (y - MASK_SIZE / 2 + dy)<IMG_HEIGHT && (x - MASK_SIZE / 2 + dx) >= 0 && (x - MASK_SIZE / 2 + dx)<IMG_WIDTH) {
								if (cTmpImg[(y - MASK_SIZE / 2 + dy)*IMG_WIDTH + (x - MASK_SIZE / 2 + dx)]<min) {
									min = cTmpImg[(y - MASK_SIZE / 2 + dy)*IMG_WIDTH + (x - MASK_SIZE / 2 + dx)];
								}
							}
						}
					}
				}
				pOutImg[y*IMG_WIDTH + x] = (LIGHTING_UINT8)min;
			}
		}
		memcpy(cTmpImg, pOutImg, sizeof(unsigned char)*IMG_WIDTH*IMG_HEIGHT);
	}

	/*
	// 線形変換
	for(y=0; y<IMG_HEIGHT; y++){
	for(x=0; x<IMG_WIDTH; x++){
	if(xmin > pOutImg[y*IMG_WIDTH+x]){
	xmin = pOutImg[y*IMG_WIDTH+x];
	}else if(xmax < pOutImg[y*IMG_WIDTH+x]){
	xmax = pOutImg[y*IMG_WIDTH+x];
	}
	}
	}
	ymin = 100;
	ymax = 200;
	a = (double)(ymax-ymin)/(xmax-xmin);
	b = (double)(xmax*ymin-xmin*ymax)/(xmax-xmin);
	for(y=0; y<IMG_HEIGHT; y++){
	for(x=0; x<IMG_WIDTH; x++){
	pOutImg[y*IMG_WIDTH+x] = a*pOutImg[y*IMG_WIDTH+x]+b;
	}
	}
	*/

	free(rTmpImg);
	free(cTmpImg);
	return (retVal);
}

/********************/
/*	Step2.gauSmooth	*/
/********************/
LIGHTING_ERROR_t gauSmooth(LIGHTING_UINT8* pInImg, LIGHTING_UINT8* pOutImg) {
#define FIL_SIZE 7
	LIGHTING_ERROR_t retVal = LIGHTING_OK;
	LIGHTING_INT32 x = 0, y = 0, fil_x = 0, fil_y = 0, fil_count = 0;
	double fil_val[FIL_SIZE*FIL_SIZE], fil_sigma = 10.0, fil_sum = 0;

	for (fil_y = -FIL_SIZE / 2; fil_y <= FIL_SIZE / 2; fil_y++) {
		for (fil_x = -FIL_SIZE / 2; fil_x <= FIL_SIZE / 2; fil_x++) {
			fil_val[fil_count] = exp(-(pow((double)fil_x, 2) + pow((double)fil_y, 2)) / (2 * pow(fil_sigma, 2)));
			fil_sum += fil_val[fil_count];
			//printf("x:%d, y:%d, val:%lf\n", fil_x, fil_y, fil_val[fil_count]);
			fil_count++;
		}
	}
	fil_count = 0;
	for (fil_y = -FIL_SIZE / 2; fil_y <= FIL_SIZE / 2; fil_y++) {
		for (fil_x = -FIL_SIZE / 2; fil_x <= FIL_SIZE / 2; fil_x++) {
			fil_val[fil_count] /= fil_sum;
			//printf("x:%d, y:%d, val:%lf\n", fil_x, fil_y, fil_val[fil_count]);
			fil_count++;
		}
	}

	/*	ガウシアンフィルタ	*/
	for (y = 0; y<IMG_HEIGHT; y++) {
		for (x = 0; x<IMG_WIDTH; x++) {
			if (y<FIL_SIZE || y>IMG_HEIGHT - FIL_SIZE || x<FIL_SIZE || x>IMG_WIDTH - FIL_SIZE) {
				pOutImg[y*IMG_WIDTH + x] = pInImg[y*IMG_WIDTH + x];
			}
			else {
#if 1
				pOutImg[y*IMG_WIDTH + x] = +fil_val[0 * FIL_SIZE + 0] * pInImg[(y - 3)*IMG_WIDTH + (x - 3)] + fil_val[0 * FIL_SIZE + 1] * pInImg[(y - 3)*IMG_WIDTH + (x - 2)] + fil_val[0 * FIL_SIZE + 2] * pInImg[(y - 3)*IMG_WIDTH + (x - 1)] + fil_val[0 * FIL_SIZE + 3] * pInImg[(y - 3)*IMG_WIDTH + x] + fil_val[0 * FIL_SIZE + 4] * pInImg[(y - 3)*IMG_WIDTH + (x + 1)] + fil_val[0 * FIL_SIZE + 5] * pInImg[(y - 3)*IMG_WIDTH + (x + 2)] + fil_val[0 * FIL_SIZE + 6] * pInImg[(y - 3)*IMG_WIDTH + (x + 3)]
					+ fil_val[1 * FIL_SIZE + 0] * pInImg[(y - 2)*IMG_WIDTH + (x - 3)] + fil_val[1 * FIL_SIZE + 1] * pInImg[(y - 2)*IMG_WIDTH + (x - 2)] + fil_val[1 * FIL_SIZE + 2] * pInImg[(y - 2)*IMG_WIDTH + (x - 1)] + fil_val[1 * FIL_SIZE + 3] * pInImg[(y - 2)*IMG_WIDTH + x] + fil_val[1 * FIL_SIZE + 4] * pInImg[(y - 2)*IMG_WIDTH + (x + 1)] + fil_val[1 * FIL_SIZE + 5] * pInImg[(y - 2)*IMG_WIDTH + (x + 2)] + fil_val[1 * FIL_SIZE + 6] * pInImg[(y - 2)*IMG_WIDTH + (x + 3)]
					+ fil_val[2 * FIL_SIZE + 0] * pInImg[(y - 1)*IMG_WIDTH + (x - 3)] + fil_val[2 * FIL_SIZE + 1] * pInImg[(y - 1)*IMG_WIDTH + (x - 2)] + fil_val[2 * FIL_SIZE + 2] * pInImg[(y - 1)*IMG_WIDTH + (x - 1)] + fil_val[2 * FIL_SIZE + 3] * pInImg[(y - 1)*IMG_WIDTH + x] + fil_val[2 * FIL_SIZE + 4] * pInImg[(y - 1)*IMG_WIDTH + (x + 1)] + fil_val[2 * FIL_SIZE + 5] * pInImg[(y - 1)*IMG_WIDTH + (x + 2)] + fil_val[2 * FIL_SIZE + 6] * pInImg[(y - 1)*IMG_WIDTH + (x + 3)]
					+ fil_val[3 * FIL_SIZE + 0] * pInImg[y*IMG_WIDTH + (x - 3)] + fil_val[3 * FIL_SIZE + 1] * pInImg[y*IMG_WIDTH + (x - 2)] + fil_val[3 * FIL_SIZE + 2] * pInImg[y*IMG_WIDTH + (x - 1)] + fil_val[3 * FIL_SIZE + 3] * pInImg[y*IMG_WIDTH + x] + fil_val[3 * FIL_SIZE + 4] * pInImg[y*IMG_WIDTH + (x + 1)] + fil_val[3 * FIL_SIZE + 5] * pInImg[y*IMG_WIDTH + (x + 2)] + fil_val[3 * FIL_SIZE + 6] * pInImg[y*IMG_WIDTH + (x + 3)]
					+ fil_val[4 * FIL_SIZE + 0] * pInImg[(y + 1)*IMG_WIDTH + (x - 3)] + fil_val[4 * FIL_SIZE + 1] * pInImg[(y + 1)*IMG_WIDTH + (x - 2)] + fil_val[4 * FIL_SIZE + 2] * pInImg[(y + 1)*IMG_WIDTH + (x - 1)] + fil_val[4 * FIL_SIZE + 3] * pInImg[(y + 1)*IMG_WIDTH + x] + fil_val[4 * FIL_SIZE + 4] * pInImg[(y + 1)*IMG_WIDTH + (x + 1)] + fil_val[4 * FIL_SIZE + 5] * pInImg[(y + 1)*IMG_WIDTH + (x + 2)] + fil_val[4 * FIL_SIZE + 6] * pInImg[(y + 1)*IMG_WIDTH + (x + 3)]
					+ fil_val[5 * FIL_SIZE + 0] * pInImg[(y + 2)*IMG_WIDTH + (x - 3)] + fil_val[5 * FIL_SIZE + 1] * pInImg[(y + 2)*IMG_WIDTH + (x - 2)] + fil_val[5 * FIL_SIZE + 2] * pInImg[(y + 2)*IMG_WIDTH + (x - 1)] + fil_val[5 * FIL_SIZE + 3] * pInImg[(y + 2)*IMG_WIDTH + x] + fil_val[5 * FIL_SIZE + 4] * pInImg[(y + 2)*IMG_WIDTH + (x + 1)] + fil_val[5 * FIL_SIZE + 5] * pInImg[(y + 2)*IMG_WIDTH + (x + 2)] + fil_val[5 * FIL_SIZE + 6] * pInImg[(y + 2)*IMG_WIDTH + (x + 3)]
					+ fil_val[6 * FIL_SIZE + 0] * pInImg[(y + 3)*IMG_WIDTH + (x - 3)] + fil_val[6 * FIL_SIZE + 1] * pInImg[(y + 3)*IMG_WIDTH + (x - 2)] + fil_val[6 * FIL_SIZE + 2] * pInImg[(y + 3)*IMG_WIDTH + (x - 1)] + fil_val[6 * FIL_SIZE + 3] * pInImg[(y + 3)*IMG_WIDTH + x] + fil_val[6 * FIL_SIZE + 4] * pInImg[(y + 3)*IMG_WIDTH + (x + 1)] + fil_val[6 * FIL_SIZE + 5] * pInImg[(y + 3)*IMG_WIDTH + (x + 2)] + fil_val[6 * FIL_SIZE + 6] * pInImg[(y + 3)*IMG_WIDTH + (x + 3)];
#else
				pOutImg[y*IMG_WIDTH + x] = +fil_val[0 * FIL_SIZE + 0] * pInImg[(y - 3)*IMG_WIDTH + (x - 3)] + fil_val[0 * FIL_SIZE + 1] * pInImg[(y - 3)*IMG_WIDTH + (x - 2)] + fil_val[0 * FIL_SIZE + 2] * pInImg[(y - 3)*IMG_WIDTH + (x - 1)] + fil_val[0 * FIL_SIZE + 3] * pInImg[(y - 3)*IMG_WIDTH + x] + fil_val[0 * FIL_SIZE + 4] * pInImg[(y - 3)*IMG_WIDTH + (x + 1)] + fil_val[0 * FIL_SIZE + 5] * pInImg[(y - 3)*IMG_WIDTH + (x + 2)] + fil_val[0 * FIL_SIZE + 6] * pInImg[(y - 3)*IMG_WIDTH + (x + 3)]
					+ fil_val[1 * FIL_SIZE + 0] * pInImg[(y - 2)*IMG_WIDTH + (x - 3)] + fil_val[1 * FIL_SIZE + 1] * pInImg[(y - 2)*IMG_WIDTH + (x - 2)] + fil_val[1 * FIL_SIZE + 2] * pInImg[(y - 2)*IMG_WIDTH + (x - 1)] + fil_val[1 * FIL_SIZE + 3] * pInImg[(y - 2)*IMG_WIDTH + x] + fil_val[1 * FIL_SIZE + 4] * pInImg[(y - 2)*IMG_WIDTH + (x + 1)] + fil_val[1 * FIL_SIZE + 5] * pInImg[(y - 2)*IMG_WIDTH + (x + 2)] + fil_val[1 * FIL_SIZE + 6] * pInImg[(y - 2)*IMG_WIDTH + (x + 3)]
					+ fil_val[2 * FIL_SIZE + 0] * pInImg[(y - 1)*IMG_WIDTH + (x - 3)] + fil_val[2 * FIL_SIZE + 1] * pInImg[(y - 1)*IMG_WIDTH + (x - 2)] + fil_val[2 * FIL_SIZE + 2] * pInImg[(y - 1)*IMG_WIDTH + (x - 1)] + fil_val[2 * FIL_SIZE + 3] * pInImg[(y - 1)*IMG_WIDTH + x] + fil_val[2 * FIL_SIZE + 4] * pInImg[(y - 1)*IMG_WIDTH + (x + 1)] + fil_val[2 * FIL_SIZE + 5] * pInImg[(y - 1)*IMG_WIDTH + (x + 2)] + fil_val[2 * FIL_SIZE + 6] * pInImg[(y - 1)*IMG_WIDTH + (x + 3)]
					+ fil_val[3 * FIL_SIZE + 0] * pInImg[y*IMG_WIDTH + (x - 3)] + fil_val[3 * FIL_SIZE + 1] * pInImg[y*IMG_WIDTH + (x - 2)] + fil_val[3 * FIL_SIZE + 2] * pInImg[y*IMG_WIDTH + (x - 1)] + fil_val[3 * FIL_SIZE + 3] * pInImg[y*IMG_WIDTH + x] + fil_val[3 * FIL_SIZE + 4] * pInImg[y*IMG_WIDTH + (x + 1)] + fil_val[3 * FIL_SIZE + 5] * pInImg[y*IMG_WIDTH + (x + 2)] + fil_val[3 * FIL_SIZE + 6] * pInImg[y*IMG_WIDTH + (x + 3)]
					+ fil_val[4 * FIL_SIZE + 0] * pInImg[(y + 1)*IMG_WIDTH + (x - 3)] + fil_val[4 * FIL_SIZE + 1] * pInImg[(y + 1)*IMG_WIDTH + (x - 2)] + fil_val[4 * FIL_SIZE + 2] * pInImg[(y + 1)*IMG_WIDTH + (x - 1)] + fil_val[4 * FIL_SIZE + 3] * pInImg[(y + 1)*IMG_WIDTH + x] + fil_val[4 * FIL_SIZE + 4] * pInImg[(y + 1)*IMG_WIDTH + (x + 1)] + fil_val[4 * FIL_SIZE + 5] * pInImg[(y + 1)*IMG_WIDTH + (x + 2)] + fil_val[4 * FIL_SIZE + 6] * pInImg[(y + 1)*IMG_WIDTH + (x + 3)]
					+ fil_val[5 * FIL_SIZE + 0] * pInImg[(y + 2)*IMG_WIDTH + (x - 3)] + fil_val[5 * FIL_SIZE + 1] * pInImg[(y + 2)*IMG_WIDTH + (x - 2)] + fil_val[5 * FIL_SIZE + 2] * pInImg[(y + 2)*IMG_WIDTH + (x - 1)] + fil_val[5 * FIL_SIZE + 3] * pInImg[(y + 2)*IMG_WIDTH + x] + fil_val[5 * FIL_SIZE + 4] * pInImg[(y + 2)*IMG_WIDTH + (x + 1)] + fil_val[5 * FIL_SIZE + 5] * pInImg[(y + 2)*IMG_WIDTH + (x + 2)] + fil_val[5 * FIL_SIZE + 6] * pInImg[(y + 2)*IMG_WIDTH + (x + 3)]
					+ fil_val[6 * FIL_SIZE + 0] * pInImg[(y + 3)*IMG_WIDTH + (x - 3)] + fil_val[6 * FIL_SIZE + 1] * pInImg[(y + 3)*IMG_WIDTH + (x - 2)] + fil_val[6 * FIL_SIZE + 2] * pInImg[(y + 3)*IMG_WIDTH + (x - 1)] + fil_val[6 * FIL_SIZE + 3] * pInImg[(y + 3)*IMG_WIDTH + x] + fil_val[6 * FIL_SIZE + 4] * pInImg[(y + 3)*IMG_WIDTH + (x + 1)] + fil_val[6 * FIL_SIZE + 5] * pInImg[(y + 3)*IMG_WIDTH + (x + 2)] + fil_val[6 * FIL_SIZE + 6] * pInImg[(y + 3)*IMG_WIDTH + (x + 3)];

#endif
			}
		}
	}

	return(retVal);
}

/********************/
/*	Step3.geoLevel	*/
/********************/
LIGHTING_ERROR_t geoLevel(LIGHTING_UINT8* pInImg, LIGHTING_UINT8* pTempImg, LIGHTING_UINT8* pOutImg) {
	LIGHTING_ERROR_t retVal = LIGHTING_OK;

	PERCOLATION_REGION_t* Gi = LIGHTING_NULL;
	LIGHTING_POINT_t point;
	LIGHTING_UINT32 level_sum = 0, Geo_Level = 0, level_sum_s = 0, Geo_Level_s = 0;
	LIGHTING_UINT32 bright, x, y, N = N_LEVEL, L, S_count = 0, B_count = 0, L_Max = 0;
	LIGHTING_UINT32 Ng = (IMG_WIDTH*IMG_HEIGHT) / N;
	double Is = 0, Ib = 0, Ds = 0, Db = 0, alpha, lambda, var_max = 0;

	FILE *L_sum, *L_ave, *L_dev, *L_alpha;
	if ((L_sum = fopen("Level_Sum.txt", "w")) == NULL) {
		exit(1);
	}
	if ((L_ave = fopen("Level_Ave.txt", "w")) == NULL) {
		exit(1);
	}
	if ((L_dev = fopen("Level_Dev.txt", "w")) == NULL) {
		exit(1);
	}
	if ((L_alpha = fopen("Level_Alpha.txt", "w")) == NULL) {
		exit(1);
	}

	time_t Time;
	struct tm *Now;
	LIGHTING_INT8 fwname[64];

	/*	メモリ確保	*/
	Gi = (PERCOLATION_REGION_t*)malloc(sizeof(PERCOLATION_REGION_t)*N_LEVEL);

	/*	メモリ初期化	*/
	memset(Gi, 0, sizeof(PERCOLATION_REGION_t)*N_LEVEL);

	/*	強度brightの値の画素をすべて取ってくる	*/
	for (bright = 0; bright<512; bright++) {
		for (y = 0; y<IMG_HEIGHT; y++) {
			for (x = 0; x<IMG_WIDTH; x++) {
				if (pTempImg[y*IMG_WIDTH + x] == bright) {
					point.x = x;
					point.y = y;
					/*	PkをGiに追加	*/
					Gi->NUMBER[Geo_Level].GiPoint.push_back(point);
					level_sum++;
				}
			}
		}
		if (level_sum >= Ng) {
			//	入力画像のレベルごとの平均と分散・標準偏差
			Gi->NUMBER[Geo_Level].sum = level_sum;
			Gi->NUMBER[Geo_Level].ave = Gi->NUMBER[Geo_Level].var = 0;
			for (level_sum = 0; level_sum<Gi->NUMBER[Geo_Level].sum; level_sum++) {
				Gi->NUMBER[Geo_Level].ave += pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x];
			}
			Gi->NUMBER[Geo_Level].ave /= Gi->NUMBER[Geo_Level].sum;

			for (level_sum = 0; level_sum<Gi->NUMBER[Geo_Level].sum; level_sum++) {
				Gi->NUMBER[Geo_Level].var += pow(pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] - Gi->NUMBER[Geo_Level].ave, 2);
			}
			Gi->NUMBER[Geo_Level].var /= Gi->NUMBER[Geo_Level].sum;

			fprintf(L_sum, "%d %d\n", Geo_Level, Gi->NUMBER[Geo_Level].sum);
			fprintf(L_ave, "%d %lf\n", Geo_Level, Gi->NUMBER[Geo_Level].ave);
			fprintf(L_dev, "%d %lf\n", Geo_Level, sqrt(Gi->NUMBER[Geo_Level].var));
			/*	Lの選別(ここをどうにかしたい)	*/
			if ((var_max < Gi->NUMBER[Geo_Level].var) && (Geo_Level < 128)) {
				var_max = Gi->NUMBER[Geo_Level].var;
				L_Max = Geo_Level;
			}

			level_sum = 0;
			Geo_Level++;
		}
	}

	//	入力画像のレベルごとの平均と分散・標準偏差(ラスト)
	Gi->NUMBER[Geo_Level].sum = level_sum;
	Gi->NUMBER[Geo_Level].ave = Gi->NUMBER[Geo_Level].var = 0;
	for (level_sum = 0; level_sum<Gi->NUMBER[Geo_Level].sum; level_sum++) {
		Gi->NUMBER[Geo_Level].ave += pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x];
	}
	Gi->NUMBER[Geo_Level].ave /= Gi->NUMBER[Geo_Level].sum;

	for (level_sum = 0; level_sum<Gi->NUMBER[Geo_Level].sum; level_sum++) {
		Gi->NUMBER[Geo_Level].var += pow(pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] - Gi->NUMBER[Geo_Level].ave, 2);
	}
	Gi->NUMBER[Geo_Level].var /= Gi->NUMBER[Geo_Level].sum;
	fprintf(L_sum, "%d %d\n", Geo_Level, Gi->NUMBER[Geo_Level].sum);
	fprintf(L_ave, "%d %lf\n", Geo_Level, Gi->NUMBER[Geo_Level].ave);
	fprintf(L_dev, "%d %lf\n", Geo_Level, sqrt(Gi->NUMBER[Geo_Level].var));
	if ((var_max < Gi->NUMBER[Geo_Level].var) && (Geo_Level < 128)) {
		var_max = Gi->NUMBER[Geo_Level].var;
		L_Max = Geo_Level;
	}

	N = Geo_Level;

	/****************************/
	/*	Step4.illumCompensate	*/
	/****************************/
	double calSum = 0;
#if 0
	L = 7 * N / 8;
	for (L = 0; L <= N; L++) {
		for (Geo_Level = 0; Geo_Level <= N; Geo_Level++) {
			for (level_sum = 0; level_sum<Gi->NUMBER[Geo_Level].GiPoint.size(); level_sum++) {
				if (Geo_Level <= L) {	/*	影領域S	*/
					Is += pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x];
					S_count++;
				}
				else {	/*	非影領域B	*/
					Ib += pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x];
					B_count++;
				}
				//			fprintf(fp, "Geo_level:%3d, number:%4d, x:%3d, y:%3d, val:%3d, isShadow:%s\n", Geo_Level, level_sum, Gi->NUMBER[Geo_Level].GiPoint[level_sum].x, Gi->NUMBER[Geo_Level].GiPoint[level_sum].y, Gi->NUMBER[Geo_Level].GiPoint[level_sum].val, (Geo_Level<L)? "TRUE" : "FALSE");
			}
		}
		Is /= S_count;
		Ib /= B_count;

		for (Geo_Level = 0; Geo_Level <= N; Geo_Level++) {
			for (level_sum = 0; level_sum<Gi->NUMBER[Geo_Level].GiPoint.size(); level_sum++) {
				if (Geo_Level <= L) {
					Ds += pow(pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] - Is, 2);
				}
				else {
					Db += pow(pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] - Ib, 2);
				}
			}
		}
		Ds = sqrt(Ds / S_count);
		Db = sqrt(Db / B_count);
		alpha = Db / Ds;
		lambda = Ib - alpha*Is;

		for (Geo_Level = 0; Geo_Level <= N; Geo_Level++) {
			for (level_sum = 0; level_sum<Gi->NUMBER[Geo_Level].GiPoint.size(); level_sum++) {
				if (Geo_Level <= L) {
					calSum = alpha*pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] + lambda;
					if (calSum >= 255) {
						calSum = 255;
					}
					pOutImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] = alpha*pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] + lambda;
				}
				else {

					pOutImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] = pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x];
				}
			}
		}

		fprintf(L_alpha, "Is:%3.2lf, Ib:%3.2lf, Ds:%3.2lf, Db:%3.2lf\nalpha:%3.2lf, lambda:%3.2lf, sum:%d, L:%3d\n\n", Is, Ib, Ds, Db, alpha, lambda, S_count+B_count, L);
		//fprintf(L_alpha, "%d %lf %lf %lf\n", L, Ds, Db, alpha);
		time(&Time);
		Now = localtime(&Time);
		sprintf(fwname, "output\\%4d_%2d_%2d_%2d_%2d_%2d_geoLevel_%3d_%3d.pgm", 1900 + Now->tm_year, Now->tm_mon + 1, Now->tm_mday, Now->tm_hour, Now->tm_min, Now->tm_sec, L_Max, L);
		write_pgm_file(pOutImg, IMG_WIDTH, IMG_HEIGHT, fwname);

		S_count = 0;
		B_count = 0;
		Is = 0;
		Ib = 0;
		Ds = 0;
		Db = 0;
	}

#else
	L = 7 * N / 8;
	for (L = 0; L <= N; L++) {
		for (Geo_Level = L+1; Geo_Level <= N; Geo_Level++) {
			for (level_sum = 0; level_sum < Gi->NUMBER[Geo_Level].sum; level_sum++) {
				/*	非影領域B	*/
				Ib += pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x];
				B_count++;
			}
		}
		Ib /= B_count;
		B_count = 0;
		for (Geo_Level = L+1; Geo_Level <= N; Geo_Level++) {
			for (level_sum = 0; level_sum < Gi->NUMBER[Geo_Level].sum; level_sum++) {
				//Db += pow(pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] - Ib, 2);
				Db += abs(pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] - Ib);
				B_count++;
			}
		}
		Db = Db / B_count;

		for (Geo_Level = L + 1; Geo_Level <= N; Geo_Level++) {
			for (level_sum = 0; level_sum < Gi->NUMBER[Geo_Level].sum; level_sum++) {
				Is = Gi->NUMBER[Geo_Level].ave;
				Ds += abs(pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] - Is);
				S_count++;
			}
		}
		Ds = Ds / S_count;
		//Db = sqrt(Db / B_count);
		LIGHTING_UINT32 Geo_Level_S = 0;
		for (Geo_Level = 0; Geo_Level <= N; Geo_Level++) {
			for (level_sum = 0; level_sum < Gi->NUMBER[Geo_Level].sum; level_sum++) {
				if (Geo_Level <= L) {
					Is = Gi->NUMBER[Geo_Level].ave;
					//Ds = sqrt(Gi->NUMBER[Geo_Level].var);
					alpha = Db / Ds;
					lambda = Ib - alpha*Is;
					calSum = alpha*pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] + lambda;
					//if (calSum >= 255) {
					//	calSum = 255;
					//}
					pOutImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] = calSum;
					//this is debug line, comment this line will reduce the process time
					//fprintf(L_alpha, "L:%3d Geo_Level:%3d Iadd: %3.2lf Ireal: %3d y:%3d x:%3d\n", L, Geo_Level, alpha*pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] + lambda, pOutImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x], Gi->NUMBER[Geo_Level].GiPoint[level_sum].y, Gi->NUMBER[Geo_Level].GiPoint[level_sum].x);
					//
				}
				else {
					pOutImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] = pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH + Gi->NUMBER[Geo_Level].GiPoint[level_sum].x];
				}
			}
		}

		//fprintf(L_alpha, "L:%3d, Is:%3.2lf, Ib:%3.2lf, Ds:%3.2lf, Db:%3.2lf\nalpha:%3.2lf, lambda:%3.2lf\n\n", L, Gi->NUMBER[Geo_Level].ave, Ib, sqrt(Gi->NUMBER[Geo_Level].var), Db, alpha, lambda);
		time(&Time);
		Now = localtime(&Time);
		sprintf(fwname, "output\\%4d_%2d_%2d_%2d_%2d_%2d_03geoLevel_%3d_%3d.pgm", 1900 + Now->tm_year, Now->tm_mon + 1, Now->tm_mday, Now->tm_hour, Now->tm_min, Now->tm_sec, L_Max, L);
		write_pgm_file(pOutImg, IMG_WIDTH, IMG_HEIGHT, fwname);

		S_count = 0;
		B_count = 0;
		Is = 0;
		Ib = 0;
		Ds = 0;
		Db = 0;
	}
#endif

	/* メモリ開放 */
	free(Gi);
	fclose(L_sum);
	fclose(L_ave);
	fclose(L_dev);
	fclose(L_alpha);

	return(retVal);
}