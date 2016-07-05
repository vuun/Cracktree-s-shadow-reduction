#define	_CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "lighting_percolation.h"

#define	IMG_WIDTH	(720) 
#define	IMG_HEIGHT	(480)

/************************/
/*	ShadowRemoval�{��	*/
/************************/

/************************/
/*	8�r�b�g�摜�ǂݍ���	*/
/************************/
void read_pgm_file(LIGHTING_UINT8* cp, LIGHTING_UINT32 width, LIGHTING_UINT32 height, LIGHTING_INT8* fname)
{
	char head[100];
	char *c;
	int w1, h1, d;									/*	�����g���ĂȂ�(�Ƃ������w�b�_�������擾���邽�߂̃_�~�[?)	*/

	FILE *fp = fopen((const char*)fname, "rb");

	do
	{
		fgets(head, 256, fp);						/*	�s�̍ŏ��̕����������łȂ�����head�ɌJ��Ԃ��ǂݍ���	*/
	}while(head[0]<'0' || head[0]>'9');

	w1 = strtol(head, &c, 10);						/*	720	*/
	h1 = strtol(c , NULL, 10);						/*	480(�X�y�[�X�t��)	*/

	fgets(head, 256, fp);

	d = strtol(head, &c, 10);						/*	255	*/

	if(fp) 
	{
		fread(cp, sizeof(char), width*height, fp);	/*	fp����1�o�C�g���̃f�[�^��width*height���擾��cp�Ɋi�[?	*/
		fclose(fp);
	} 
	else
	{
		printf("can't open file in write_pgm_file(%s).\n", fname);
	}
}

/************************/
/*	8�r�b�g�摜��������	*/
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
/*	8�r�b�g�摜�������݃e�X�g�v���O����	*/
/****************************************/
void main(){
	LIGHTING_UINT8* prImage = NULL;
	LIGHTING_UINT8*	pwImage = NULL;
	LIGHTING_ERROR_t retVal = LIGHTING_OK;
	LIGHTING_INT8 fname[64];

	LIGHTING_UINT32 ImgWidth = IMG_WIDTH;
	LIGHTING_UINT32 ImgHeight = IMG_HEIGHT;
	
	LIGHTING_UINT8 N = N_LEVEL;
	time_t Time;
	struct tm *Now;

	/*	������(��f����)�̊m��	*/
	prImage = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)* ImgWidth*ImgHeight);
	pwImage = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)* ImgWidth*ImgHeight);

	/*	������(��f����)�̏�����	*/
	memset(prImage, 0, sizeof(unsigned char)*ImgWidth*ImgHeight);
	memset(pwImage, 0, sizeof(unsigned char)*ImgWidth*ImgHeight);

	sprintf(fname, "2012-12-11 10.45.58_720_480.pgm");
	read_pgm_file(prImage, ImgWidth, ImgHeight, fname);

	retVal = gauSmooth(prImage, pwImage);
	retVal = geoLevel(prImage, pwImage, &N);

	time(&Time);
	Now = localtime(&Time);
	sprintf(fname, "C:\\temp\\result\\%dy%dm%dd%dh%dm%ds_geoLevel_%d.pgm", 1900+Now->tm_year, Now->tm_mon+1, Now->tm_mday, Now->tm_hour, Now->tm_min, Now->tm_sec, N);
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
	/*	LIGHTING_UINT8* pInImg		���͉摜(I)		*/
	/*	LIGHTING_UINT8* pOutImg		�o�͉摜(I/O)	*/
	/************************************************/
{
	LIGHTING_ERROR_t retVal = LIGHTING_OK;
	LIGHTING_INT32 x = 0, y = 0;

	/*	�K�E�V�A���t�B���^	*/
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
LIGHTING_ERROR_t geoLevel(LIGHTING_UINT8* pInImg, LIGHTING_UINT8* pOutImg, LIGHTING_UINT8* N)
	/************************************************/
	/*	LIGHTING_UINT8* pInImg		���͉摜(I)		*/
	/*	LIGHTING_UINT8* pOutImg		�o�͉摜(I/O)	*/
	/************************************************/
{
	LIGHTING_ERROR_t retVal = LIGHTING_OK;

	PERCOLATION_REGION_t* Gi = LIGHTING_NULL;
	LIGHTING_INT32 level_sum = 0, Geo_Level = 0;
	LIGHTING_INT32 bright, x, y, count, L, shadow_cnt = 0, nshadow_cnt = 0;
	LIGHTING_INT32 Ng = (IMG_WIDTH*IMG_HEIGHT)/(*N);
	double ave1 = 0, ave2 = 0, var1 = 0, var2 = 0, alpha, lambda;

	FILE *fp;
	if((fp=fopen("geoLevel_Result.txt", "w"))==NULL){
		exit(1);
	}
	
	/*	�������m��	*/
	Gi = (PERCOLATION_REGION_t*)malloc(sizeof(PERCOLATION_REGION_t)*GEOLEVEL_NUM);

	/*	������������	*/
	memset(Gi, 0, sizeof(PERCOLATION_REGION_t)*GEOLEVEL_NUM);

	for(bright=0; bright<256; bright++){
		for(y=0; y<IMG_HEIGHT; y++){
			for(x=0; x<IMG_WIDTH; x++){
				if(pInImg[y*IMG_WIDTH+x] == bright){
					Gi->NUMBER[Geo_Level].point[level_sum].x = x;
					Gi->NUMBER[Geo_Level].point[level_sum].y = y;
					Gi->NUMBER[Geo_Level].val = pInImg[y*IMG_WIDTH+x];
					fprintf(fp, "Level:%3d, number:%4d, x:%3d, y:%3d, val:%3d\n", Geo_Level, level_sum, Gi->NUMBER[Geo_Level].point[level_sum].x, Gi->NUMBER[Geo_Level].point[level_sum].y, Gi->NUMBER[Geo_Level].val);
					pOutImg[y*IMG_WIDTH+x] = Geo_Level;
					level_sum++;
				}
			}
		}

		Gi->NUMBER[Geo_Level].sum = level_sum;
		if(level_sum >= Ng){
			level_sum = 0;
			Geo_Level++;
		}
	}
	*N = Geo_Level;
	L = 7*(*N)/8;
	

	for(Geo_Level=0; Geo_Level<=*N; Geo_Level++){
		level_sum = Gi->NUMBER[Geo_Level].sum;
		for(count=0; count<level_sum; count++){
			if(Geo_Level<L){
				Gi->NUMBER[Geo_Level].isShadow = 1;
				ave1 += pInImg[Gi->NUMBER[Geo_Level].point[count].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].point[count].x];
				shadow_cnt++;
			}
			else{
				Gi->NUMBER[Geo_Level].isShadow = 0;
				ave2 += pInImg[Gi->NUMBER[Geo_Level].point[count].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].point[count].x];
				nshadow_cnt++;
			}
			fprintf(fp, "Geo_level:%3d, number:%4d, x:%3d, y:%3d, val:%3d, isShadow:%s\n", Geo_Level, count, Gi->NUMBER[Geo_Level].point[count].x, Gi->NUMBER[Geo_Level].point[count].y, Gi->NUMBER[Geo_Level].val, (Gi->NUMBER[Geo_Level].isShadow==1)? "TRUE" : "FALSE");
		}
	}
	ave1/=shadow_cnt;
	ave2/=nshadow_cnt;

	for(Geo_Level=0; Geo_Level<=*N; Geo_Level++){
		level_sum = Gi->NUMBER[Geo_Level].sum;
		for(count=0; count<level_sum; count++){
			if(Geo_Level<L){
				var1 += pow(pInImg[Gi->NUMBER[Geo_Level].point[count].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].point[count].x]-ave1, 2);
			}
			else{
				var2 += pow(pInImg[Gi->NUMBER[Geo_Level].point[count].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].point[count].x]-ave2, 2);
			}
		}
	}
	var1/=shadow_cnt;
	var2/=nshadow_cnt;
	alpha = sqrt(var2)/sqrt(var1);
	lambda = ave2-alpha*ave1;
	fprintf(fp, "ave1:%3.2lf, ave2:%3.2lf, var1:%3.2lf, var2:%3.2lf\nalpha:%3.2lf, lambda:%3.2lf\n", ave1, ave2, var1, var2, alpha, lambda);

	for(Geo_Level=0; Geo_Level<=*N; Geo_Level++){
		level_sum = Gi->NUMBER[Geo_Level].sum;
		for(count=0; count<level_sum; count++){
			if(Geo_Level<L){
				pOutImg[Gi->NUMBER[Geo_Level].point[count].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].point[count].x] = alpha*pInImg[Gi->NUMBER[Geo_Level].point[count].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].point[count].x]+lambda;
			}
			else{
				pOutImg[Gi->NUMBER[Geo_Level].point[count].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].point[count].x] = pInImg[Gi->NUMBER[Geo_Level].point[count].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].point[count].x];
			}
		}
	}


	/* �������J�� */
	free(Gi);
	fclose(fp);

	return(retVal);
}