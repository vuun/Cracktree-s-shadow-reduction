/*****************************************************************************************
非等方拡散フィルタ（Perona-Malik拡散法）ヘッダファイル。
Ver 1.00/2013/02/03 松島　新規作成。

All Rights ReservedCopyright (C), Kousuke Matsushima
******************************************************************************************/
#ifndef	LIGHTING_PMD_INCLUDED
#define	LIGHTING_PMD_INCLUDED

#include "lightning_types.h"

/* 各種パラメータ。*/
#define		Epsilon		( 0.1 )
#define		T			( 5 )			/* 正規化定数。*/
#define		LAMBDA		( 1.0 )
#define		SIGMA		( 2 )
#define		GAUSSIAN_C	( 0.130801f )	/* 中心。*/
#define		GAUSSIAN_N	( 0.115432f )	/* 周辺。*/

/*************************************************************
	関数のプロトタイプ宣言
****************************************************************/
/*********************************
 非等方拡散法。
 *********************************/
LIGHTING_ERROR_t AnisotropicDiffusion(
		LIGHTING_UINT8 *	pInImg,		/* 入力画像。*/
		LIGHTING_UINT8 *	pOutImg,	/* 出力画像。*/
		LIGHTING_UINT32		ImgWidth,	/* 画像幅。*/
		LIGHTING_UINT32		ImgHeight,	/* 画像高さ。*/
		LIGHTING_INT32		iter );		/* 繰り返し関数。*/

/*********************************
 エッジ停止関数。
 *********************************/
float EdgeStopping( 
	float diffVal,		/* グラジエント値。*/
	float kappa );		/* K値。*/

#endif