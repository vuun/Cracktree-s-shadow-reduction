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
	LIGHTING_INT8 frname[64], fwname[64];

	LIGHTING_UINT32 ImgWidth = IMG_WIDTH;
	LIGHTING_UINT32 ImgHeight = IMG_HEIGHT;
	
	time_t Time;
	struct tm *Now;

	/*	メモリ(画素数分)の確保	*/
	prImage = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)*ImgWidth*ImgHeight);
	pwImage = (LIGHTING_UINT8*)malloc(sizeof(LIGHTING_UINT8)*ImgWidth*ImgHeight);

	/*	メモリ(画素数分)の初期化	*/
	memset(prImage, 0, sizeof(unsigned char)*ImgWidth*ImgHeight);
	memset(pwImage, 0, sizeof(unsigned char)*ImgWidth*ImgHeight);

	sprintf(frname, "road01.pgm");
	read_pgm_file(prImage, ImgWidth, ImgHeight, frname);

	retVal = gauSmooth(prImage, pwImage);
	retVal = geoLevel(prImage, pwImage);

	time(&Time);
	Now = localtime(&Time);
	sprintf(fwname, "..\\Result\\%4d_%2d_%2d_%2d_%2d_%2d_geoLevel_%d_%s", 1900+Now->tm_year, Now->tm_mon+1, Now->tm_mday, Now->tm_hour, Now->tm_min, Now->tm_sec, retVal,  frname);
	write_pgm_file(pwImage, ImgWidth, ImgHeight, fwname);

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

/************************************/
/*	geoLevel(及びillumCompensate)	*/
/************************************/
LIGHTING_ERROR_t geoLevel(LIGHTING_UINT8* pInImg, LIGHTING_UINT8* pOutImg)
	/************************************************/
	/*	LIGHTING_UINT8* pInImg		入力画像(I)		*/
	/*	LIGHTING_UINT8* pOutImg		出力画像(I/O)	*/
	/************************************************/
{
	LIGHTING_ERROR_t retVal = LIGHTING_OK;

	PERCOLATION_REGION_t* Gi = LIGHTING_NULL;
	LIGHTING_POINT_t point;
	LIGHTING_UINT32 level_sum = 0, Geo_Level = 0;
	LIGHTING_UINT32 bright, x, y, N = N_LEVEL, L, S_count = 0, B_count = 0;
	LIGHTING_UINT32 Ng = (IMG_WIDTH*IMG_HEIGHT)/N;
	double Is = 0, Ib = 0, Ds = 0, Db = 0, alpha, lambda;

	FILE *fp;
	if((fp=fopen("geoLevel_Result.txt", "w"))==NULL){
		exit(1);
	}
	
	/*	メモリ確保	*/
	Gi = (PERCOLATION_REGION_t*)malloc(sizeof(PERCOLATION_REGION_t)*N_LEVEL);

	/*	メモリ初期化	*/
	memset(Gi, 0, sizeof(PERCOLATION_REGION_t)*N_LEVEL);

	/*	強度brightの値の画素をすべて取ってくる	*/
	for(bright=0; bright<250; bright++){
		for(y=0; y<IMG_HEIGHT; y++){
			for(x=0; x<IMG_WIDTH; x++){
				if(pInImg[y*IMG_WIDTH+x] == bright){
					point.x = x;
					point.y = y;
					point.val = bright;
					/*	PkをGiに追加	*/
					Gi->NUMBER[Geo_Level].GiPoint.push_back(point);

//					fprintf(fp, "Geo_level:%3d, size:%4d, x:%3d, y:%3d, val:%3d\n", Geo_Level, Gi->NUMBER[Geo_Level].GiPoint.size(), Gi->NUMBER[Geo_Level].GiPoint[level_sum].x, Gi->NUMBER[Geo_Level].GiPoint[level_sum].y, Gi->NUMBER[Geo_Level].GiPoint[level_sum].val);

					level_sum++;
				}
			}
		}

		if(level_sum >= Ng){
			level_sum = 0;
			Geo_Level++;
		}
	}

	N = Geo_Level;
	
	/*	分散比alphaを無理やり1に近づける	*/
	for(L=0; L<256; L++){
		/*	ここからillumCompensate	*/
		for(Geo_Level=0; Geo_Level<=N; Geo_Level++){
			for(level_sum=0; level_sum<Gi->NUMBER[Geo_Level].GiPoint.size(); level_sum++){
				if(Geo_Level<L){	/*	影領域S	*/
					Is += Gi->NUMBER[Geo_Level].GiPoint[level_sum].val;
					S_count++;
				}
				else{	/*	非影領域B	*/
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

		fprintf(fp, "L:%3d, alpha:%lf\n", L, alpha);
		if(alpha<1.10 && alpha>0.90){
			break;
		}
		
		S_count = 0;
		B_count = 0;
		Is = 0;
		Ib = 0;
		Ds = 0;
		Db = 0;
	}
	
	for(Geo_Level=0; Geo_Level<=N; Geo_Level++){
		for(level_sum=0; level_sum<Gi->NUMBER[Geo_Level].GiPoint.size(); level_sum++){
			if(Geo_Level<L){
				pOutImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] = alpha*Gi->NUMBER[Geo_Level].GiPoint[level_sum].val+lambda;
			}
			else{
				pOutImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].GiPoint[level_sum].x] = Gi->NUMBER[Geo_Level].GiPoint[level_sum].val;
			}

//			fprintf(fp, "Geo_level:%3d, size:%4d, x:%3d, y:%3d, val:%3d\n", Geo_Level, level_sum, Gi->NUMBER[Geo_Level].GiPoint[level_sum].x, Gi->NUMBER[Geo_Level].GiPoint[level_sum].y, pOutImg[Gi->NUMBER[Geo_Level].GiPoint[level_sum].y*IMG_WIDTH+Gi->NUMBER[Geo_Level].GiPoint[level_sum].x]);
		}
	}

	fprintf(fp, "Is:%3.2lf, Ib:%3.2lf, Ds:%3.2lf, Db:%3.2lf\nalpha:%3.2lf, lambda:%3.2lf, sum:%d\n", Is, Ib, Ds, Db, alpha, lambda, S_count+B_count);

	/* メモリ開放 */
	free(Gi);
	fclose(fp);

	return(alpha*100);
}