#ifndef	LIGHTING_PERCOLATION_INCLUDED
	#define	LIGHTING_PERCOLATION_INCLUDED

	#include "lightning_types.h"
	#include <vector>

	#define N_LEVEL					(240)		/* Step3で用いる画素数の閾値Tc:5 */

	/************************/
	/*	浸透領域の構造体	*/
	/************************/
	typedef struct tag_PERCOLATION_REGION
	{
		struct
		{
			std::vector <LIGHTING_POINT_t> GiPoint;
			LIGHTING_UINT32  sum;						/*濃度値*/
			double ave, var;
		}NUMBER[N_LEVEL];
	}PERCOLATION_REGION_t;

	/****************************/
	/*	関数のプロトタイプ宣言	*/
	/****************************/
	void read_pgm_file(LIGHTING_UINT8* cp, LIGHTING_UINT32 width, LIGHTING_UINT32 height, LIGHTING_INT8* fname);
	void write_pgm_file(LIGHTING_UINT8* cp, LIGHTING_UINT32 width, LIGHTING_UINT32 height, LIGHTING_INT8* fname);
	LIGHTING_ERROR_t mmClose(LIGHTING_UINT8* pInImg, LIGHTING_UINT8* pOutImg);
	LIGHTING_ERROR_t gauSmooth(LIGHTING_UINT8* pInImg, LIGHTING_UINT8* pOutImg);
	LIGHTING_ERROR_t geoLevel(LIGHTING_UINT8* pInImg, LIGHTING_UINT8* pTempImg, LIGHTING_UINT8* pOutImg);
#endif