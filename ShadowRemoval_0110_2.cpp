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
	LIGHTING_UINT8* readImg = NULL;
	LIGHTING_UINT8*	mmCloseImg = NULL;
	LIGHTING_UINT8* gauSmoothImg = NULL;
	LIGHTING_UINT8*	geoLevelImg = NULL;
	LIGHTING_ERROR_t retVal = LIGHTING_OK;
	LIGHTING_INT8 frname[64], fwname[64];

	time_t Time;
	struct tm *Now;

	/*	������(��f����)�̊m��	*/
	readImg = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)*IMG_WIDTH*IMG_HEIGHT);
	mmCloseImg = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)*IMG_WIDTH*IMG_HEIGHT);
	gauSmoothImg = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)*IMG_WIDTH*IMG_HEIGHT);
	geoLevelImg = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)*IMG_WIDTH*IMG_HEIGHT);
	/*	������(��f����)�̏�����	*/
	memset(readImg, 0, sizeof(unsigned char)*IMG_WIDTH*IMG_HEIGHT);
	memset(mmCloseImg, 0, sizeof(unsigned char)*IMG_WIDTH*IMG_HEIGHT);
	memset(gauSmoothImg, 0, sizeof(unsigned char)*IMG_WIDTH*IMG_HEIGHT);
	memset(geoLevelImg, 0, sizeof(unsigned char)*IMG_WIDTH*IMG_HEIGHT);

	sprintf(frname, "road01.pgm");
	read_pgm_file(readImg, IMG_WIDTH, IMG_HEIGHT, frname);

	retVal = mmClose(readImg, mmCloseImg);
	time(&Time);
	Now = localtime(&Time);
	sprintf(fwname, "..\\Result\\%4d_%2d_%2d_%2d_%2d_%2d_01_mmClose_%s", 1900+Now->tm_year, Now->tm_mon+1, Now->tm_mday, Now->tm_hour, Now->tm_min, Now->tm_sec, frname);
	write_pgm_file(mmCloseImg, IMG_WIDTH, IMG_HEIGHT, fwname);

	retVal = gauSmooth(mmCloseImg, gauSmoothImg);
	time(&Time);
	Now = localtime(&Time);
	sprintf(fwname, "..\\Result\\%4d_%2d_%2d_%2d_%2d_%2d_02_gauSmooth_%s", 1900+Now->tm_year, Now->tm_mon+1, Now->tm_mday, Now->tm_hour, Now->tm_min, Now->tm_sec, frname);
	write_pgm_file(gauSmoothImg, IMG_WIDTH, IMG_HEIGHT, fwname);

	retVal = geoLevel(readImg, gauSmoothImg, geoLevelImg);
	time(&Time);
	Now = localtime(&Time);
	sprintf(fwname, "..\\Result\\%4d_%2d_%2d_%2d_%2d_%2d_03_geoLevel_%d_%s", 1900+Now->tm_year, Now->tm_mon+1, Now->tm_mday, Now->tm_hour, Now->tm_min, Now->tm_sec, retVal,  frname);
	write_pgm_file(geoLevelImg, IMG_WIDTH, IMG_HEIGHT, fwname);

	free(readImg);
	free(mmCloseImg);
	free(gauSmoothImg);
	free(geoLevelImg);
	return;
}

/************/
/*	mmClose	*/
/************/
LIGHTING_ERROR_t mmClose(LIGHTING_UINT8* pInImg, LIGHTING_UINT8* pOutImg)
	/************************************************/
	/*	LIGHTING_UINT8* pInImg		���͉摜(I)		*/
	/*	LIGHTING_UINT8* pOutImg		�o�͉摜(I/O)	*/
	/************************************************/
{
	LIGHTING_ERROR_t retVal = LIGHTING_OK;

	LIGHTING_INT32 x, y, dx, dy, max=255, min=0, rows=3, cols=3;
	LIGHTING_UINT8* pTmpImg = LIGHTING_NULL;
/*	LIGHTING_INT32 mask[5][5] = {
		{0, 0, 1, 0, 0},
		{0, 1, 1, 1, 0},
		{1, 1, 1, 1, 1},
		{0, 1, 1, 1, 0},
		{0, 0, 1, 0, 0}
	};
*/
	LIGHTING_INT32 mask[3][3] = {
		{0, 1, 0},
		{1, 1, 1},
		{0, 1, 0}
	};

	/*	�������m��	*/
	pTmpImg = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)*IMG_WIDTH*IMG_HEIGHT);
	/*	������������	*/
	memset(pTmpImg, 0, sizeof(unsigned char)*IMG_WIDTH*IMG_HEIGHT);
	memcpy(pTmpImg, pInImg, sizeof(unsigned char)*IMG_WIDTH*IMG_HEIGHT);

#if 1
	/*	�c��	*/
	for(y=1; y<IMG_HEIGHT; y++){
		for(x=1; x<IMG_WIDTH; x++){
			max = 0;		
			for(dy=0; dy<rows; dy++){			/* �\���v�f�����B*/
				for(dx=0; dx<cols; dx++){		/* �\���v�f���B*/
					if(mask[dy][dx]==1){
						if(pTmpImg[(y+dy)*IMG_WIDTH+(x+dx)]>max){
							max = pTmpImg[(y+dy)*IMG_WIDTH+(x+dx)];
						}
					}
				}
			}			
			pTmpImg[y*IMG_WIDTH+x] = (LIGHTING_UINT8)max;		
		}
	}
#endif

#if 1
	/*	���k	*/
	for(y=1; y<IMG_HEIGHT; y++){
		for(x=1; x<IMG_WIDTH; x++){
			min = 255;		
			for(dy=0; dy<rows; dy++){			/* �\���v�f�����B*/
				for(dx=0; dx<cols; dx++){		/* �\���v�f���B*/
					if(mask[dy][dx]==1){
						if(pTmpImg[(y+dy)*IMG_WIDTH+(x+dx)]<min){
							min = pTmpImg[(y+dy)*IMG_WIDTH+(x+dx)];
						}
					}
				}
			}			
			pOutImg[y*IMG_WIDTH+x] = (LIGHTING_UINT8)min;		
		}
	}
#endif
	return (retVal);
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

/************************************/
/*	geoLevel(�y��illumCompensate)	*/
/************************************/
LIGHTING_ERROR_t geoLevel(LIGHTING_UINT8* pInImg, LIGHTING_UINT8* pTempImg, LIGHTING_UINT8* pOutImg)
	/************************************************/
	/*	LIGHTING_UINT8* pInImg		���͉摜(I)		*/
	/*	LIGHTING_UINT8* pOutImg		�o�͉摜(I/O)	*/
	/************************************************/
{
	LIGHTING_ERROR_t retVal = LIGHTING_OK;

	PERCOLATION_REGION_t* Gi = LIGHTING_NULL;
	LIGHTING_POINT_t point;
	LIGHTING_UINT32 level_sum = 0, Geo_Level = 0, level_sum2 = 0, Geo_Level2 = 0;
	LIGHTING_UINT32 bright, x, y, N = N_LEVEL, L, S_count = 0, B_count = 0;
	LIGHTING_UINT32 Ng = (IMG_WIDTH*IMG_HEIGHT)/N;
	double Is = 0, Ib = 0, Ds = 0, Db = 0, alpha, lambda, dev, ave;

	FILE *fp;
	if((fp=fopen("geoLevel_Result.txt", "w"))==NULL){
		exit(1);
	}
	
	/*	�������m��	*/
	Gi = (PERCOLATION_REGION_t*)malloc(sizeof(PERCOLATION_REGION_t)*N_LEVEL);

	/*	������������	*/
	memset(Gi, 0, sizeof(PERCOLATION_REGION_t)*N_LEVEL);

	/*	���xbright�̒l�̉�f�����ׂĎ���Ă���	*/
	for(bright=0; bright<256; bright++){
		for(y=0; y<IMG_HEIGHT; y++){
			for(x=0; x<IMG_WIDTH; x++){
				if(pTempImg[y*IMG_WIDTH+x] == bright){
					point.x = x;
					point.y = y;
					point.val = bright;
					/*	Pk��Gi�ɒǉ�	*/
					Gi->NUMBER[Geo_Level].GiPoint.push_back(point);

//					fprintf(fp, "Geo_level:%3d, size:%4d, x:%3d, y:%3d, val:%3d\n", Geo_Level, Gi->NUMBER[Geo_Level].GiPoint.size(), Gi->NUMBER[Geo_Level].GiPoint[level_sum].x, Gi->NUMBER[Geo_Level].GiPoint[level_sum].y, Gi->NUMBER[Geo_Level].GiPoint[level_sum].val);

					level_sum++;
				}
			}
		}

		if(level_sum >= Ng){
#if 0
			for(level_sum2=0; level_sum2<level_sum; level_sum2++){
				ave+= Gi->NUMBER[Geo_Level].GiPoint[level_sum2].val;
			}
			ave /= level_sum;

			for(level_sum2=0; level_sum2<level_sum; level_sum2++){
				dev+= pow(Gi->NUMBER[Geo_Level].GiPoint[level_sum2].val-ave, 2);
			}
			dev /= level_sum;
			fprintf(fp, "%d %lf\n", Geo_Level, sqrt(dev));
#endif
			level_sum = 0;
			Geo_Level++;
		}
	}

	N = Geo_Level;
	L = 7*N/8;

#if 1
	/*	���U��alpha�𖳗����1�ɋ߂Â���	*/
	for(L=255; L>=0; L--){
#endif
		/*	��������illumCompensate	*/
		for(Geo_Level=0; Geo_Level<=N; Geo_Level++){
			for(level_sum=0; level_sum<Gi->NUMBER[Geo_Level].GiPoint.size(); level_sum++){
				if(Geo_Level<L){	/*	�e�̈�S	*/
					Is += Gi->NUMBER[Geo_Level].GiPoint[level_sum].val;
					S_count++;
				}
				else{	/*	��e�̈�B	*/
					Ib += Gi->NUMBER[Geo_Level].GiPoint[level_sum].val;
					B_count++;
				}
	//			fprintf(fp, "Geo_level:%3d, number:%4d, x:%3d, y:%3d, val:%3d, isShadow:%s\n", Geo_Level, level_sum, Gi->NUMBER[Geo_Level].GiPoint[level_sum].x, Gi->NUMBER[Geo_Level].GiPoint[level_sum].y, Gi->NUMBER[Geo_Level].GiPoint[level_sum].val, (Geo_Level<L)? "TRUE" : "FALSE");
			}
		}
		Is/=S_count;
		Ib/=B_count;

		for(Geo_Level=0; Geo_Level<=N; Geo_Level++){
			for(level_sum=0; level_sum<Gi->NUMBER[Geo_Level].GiPoint.size(); level_sum++){
				if(Geo_Level<L){
					Ds += pow(Gi->NUMBER[Geo_Level].GiPoint[level_sum].val-Is, 2);
				}
				else{
					Db += pow(Gi->NUMBER[Geo_Level].GiPoint[level_sum].val-Ib, 2);
				}
			}
		}
		Ds=sqrt(Ds/S_count);
		Db=sqrt(Db/B_count);
		alpha = Db/Ds;
		lambda = Ib-alpha*Is;
#if 1
		fprintf(fp, "L:%3d, alpha:%lf\n", L, alpha);
		if(alpha>0.90){
			break;
		}
		
		S_count = 0;
		B_count = 0;
		Is = 0;
		Ib = 0;
		Ds = 0;
		Db = 0;
	}
#endif

	for(Geo_Level=0; Geo_Level<=N; Geo_Level++){
		for(level_sum=0; level_sum<Gi->NUMBER[Geo_Level].GiPoint.size(); level_sum++){
			if(Geo_Level<L){
				pOutImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] = alpha*pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].GiPoint[level_sum].x]+lambda;
			}
			else{
				pOutImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] = pInImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].GiPoint[level_sum].x];
			}

//			fprintf(fp, "Geo_level:%3d, size:%4d, x:%3d, y:%3d, val:%3d\n", Geo_Level, level_sum, Gi->NUMBER[Geo_Level].GiPoint[level_sum].x, Gi->NUMBER[Geo_Level].GiPoint[level_sum].y, pOutImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].GiPoint[level_sum].x]);
		}
	}

	fprintf(fp, "Is:%3.2lf, Ib:%3.2lf, Ds:%3.2lf, Db:%3.2lf\nalpha:%3.2lf, lambda:%3.2lf, sum:%d, L:%3d\n", Is, Ib, Ds, Db, alpha, lambda, S_count+B_count, L);

	/* �������J�� */
	free(Gi);
	fclose(fp);

	return(alpha*100);
}