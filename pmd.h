/*****************************************************************************************
�񓙕��g�U�t�B���^�iPerona-Malik�g�U�@�j�w�b�_�t�@�C���B
Ver 1.00/2013/02/03 �����@�V�K�쐬�B

All Rights ReservedCopyright (C), Kousuke Matsushima
******************************************************************************************/
#ifndef	LIGHTING_PMD_INCLUDED
#define	LIGHTING_PMD_INCLUDED

#include "lightning_types.h"

/* �e��p�����[�^�B*/
#define		Epsilon		( 0.1 )
#define		T			( 5 )			/* ���K���萔�B*/
#define		LAMBDA		( 1.0 )
#define		SIGMA		( 2 )
#define		GAUSSIAN_C	( 0.130801f )	/* ���S�B*/
#define		GAUSSIAN_N	( 0.115432f )	/* ���ӁB*/

/*************************************************************
	�֐��̃v���g�^�C�v�錾
****************************************************************/
/*********************************
 �񓙕��g�U�@�B
 *********************************/
LIGHTING_ERROR_t AnisotropicDiffusion(
		LIGHTING_UINT8 *	pInImg,		/* ���͉摜�B*/
		LIGHTING_UINT8 *	pOutImg,	/* �o�͉摜�B*/
		LIGHTING_UINT32		ImgWidth,	/* �摜���B*/
		LIGHTING_UINT32		ImgHeight,	/* �摜�����B*/
		LIGHTING_INT32		iter );		/* �J��Ԃ��֐��B*/

/*********************************
 �G�b�W��~�֐��B
 *********************************/
float EdgeStopping( 
	float diffVal,		/* �O���W�G���g�l�B*/
	float kappa );		/* K�l�B*/

#endif