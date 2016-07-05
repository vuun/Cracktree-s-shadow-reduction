#define	_CRT_SECURE_NO_WARNINGS
#define	_CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lighting_percolation.h"

#define	IMG_WIDTH	(720) 
#define	IMG_HEIGHT	(480)

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
	}while(head[0]<'0' || head[0]>'9');

	w1 = strtol(head, &c, 10);						/*	720	*/
	h1 = strtol(c , NULL, 10);						/*	480(スペース付き)	*/

	fgets(head, 256, fp);

	d = strtol(head, &c, 10);						/*	255	*/

	if(fp) 
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
	FILE *fp = fopen(fname , "wb");

	if(fp)
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
void main(){
	LIGHTING_UINT8* prImage = NULL;
	LIGHTING_UINT8*	pwImage = NULL;
	LIGHTING_ERROR_t retVal = LIGHTING_OK;

	LIGHTING_INT8 fname[64];

	LIGHTING_UINT32 ImgWidth = IMG_WIDTH;
	LIGHTING_UINT32 ImgHeight = IMG_HEIGHT;

	time_t Time;
	struct tm *Now;

	/*	メモリ(画素数分)の確保	*/
	prImage = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)* ImgWidth*ImgHeight);
	pwImage = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)* ImgWidth*ImgHeight);

	/*	メモリ(画素数分)の初期化	*/
	memset(prImage, 0, sizeof(unsigned char)*ImgWidth*ImgHeight);
	memset(pwImage, 0, sizeof(unsigned char)*ImgWidth*ImgHeight);

	sprintf(fname, "2012-12-11 10.46.31_720_480.pgm");
	read_pgm_file(prImage, ImgWidth, ImgHeight, fname);

	retVal = gauSmooth(prImage, pwImage);
	retVal = geoLevel(prImage, pwImage);

	time(&Time);
	Now = localtime(&Time);
	sprintf(fname, "C:\\temp\\result\\%d%d%d_%dh%dm%ds_gauSmooth.pgm", 1900+Now->tm_year, Now->tm_mon+1, Now->tm_mday, Now->tm_hour, Now->tm_min, Now->tm_sec);
	write_pgm_file(pwImage, ImgWidth, ImgHeight, fname);

	free(prImage);
	free(pwImage);

	return;
}

/****************/
/*	gauSmooth	*/
/****************/
LIGHTING_ERROR_t gauSmooth(LIGHTING_UINT8* pInImg, LIGHTING_UINT8* pOutImg)
	/************************************************/
	/*	LIGHTING_UINT8* pInImg		入力画像(I)		*/
	/*	LIGHTING_UINT8* pOutImg		出力画像(I/O)	*/
	/************************************************/
{
	LIGHTING_ERROR_t retVal = LIGHTING_OK;
	LIGHTING_INT32 x = 0, y = 0;

	/*	ガウシアンフィルタ	*/
	for(y=0; y<IMG_HEIGHT; y++){
		for(x=0; x<IMG_WIDTH; x++){
			if(y==0 || y==IMG_HEIGHT-1 || x==0 || x==IMG_WIDTH-1){
				pOutImg[y*IMG_WIDTH+x] = pInImg[y*IMG_WIDTH+x];
			}else{
				pOutImg[y*IMG_WIDTH+x] = (	pInImg[(y-1)*IMG_WIDTH+(x-1)]	+2*pInImg[(y-1)*IMG_WIDTH+x]	+pInImg[(y-1)*IMG_WIDTH+(x+1)]
											+2*pInImg[y*IMG_WIDTH+(x-1)]	+4*pInImg[y*IMG_WIDTH+x]		+2*pInImg[y*IMG_WIDTH+(x+1)]
											+pInImg[(y+1)*IMG_WIDTH+(x-1)]	+2*pInImg[(y+1)*IMG_WIDTH+x]	+pInImg[(y+1)*IMG_WIDTH+(x+1)]	)/16;
			}
		}
	}
	return(retVal);
}

/****************/
/*	geoLevel	*/
/****************/
LIGHTING_ERROR_t geoLevel(LIGHTING_UINT8* pInImg, LIGHTING_UINT8* pOutImg)
	/************************************************/
	/*	LIGHTING_UINT8* pInImg		入力画像(I)		*/
	/*	LIGHTING_UINT8* pOutImg		出力画像(I/O)	*/
	/************************************************/
{
	LIGHTING_ERROR_t retVal = LIGHTING_OK;
	PERCOLATION_REGION_t* pPerInf = LIGHTING_NULL;
	LIGHTING_UINT8* pCtrImage = LIGHTING_NULL;

	/*	メモリ確保	*/
	pPerInf = (PERCOLATION_REGION_t*)malloc(sizeof(PERCOLATION_REGION_t)*PERCOLATION_REGION_NUM);
	pCtrImage = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)*IMG_WIDTH*IMG_HEIGHT);

	/*	メモリ初期化	*/
	memset(pPerInf, 0, sizeof(PERCOLATION_REGION_t)*PERCOLATION_REGION_NUM);
	memset(pCtrImage, 0, sizeof(LIGHTING_UINT8)*IMG_WIDTH*IMG_HEIGHT);



	/* メモリ開放 */
	free(pCtrImage);
	free(pPerInf);

	return(retVal);
}