/*****************************************************************************************
3-D image analysis code program
Ver 1.00/2008/06/17 松島　新規作成。
All Rights ReservedCopyright (C), Kousuke Matsushima
******************************************************************************************/
#ifndef	LIGHTING_TYPES_INCLUDED
	#define	LIGHTING_TYPES_INCLUDED

	/*
	#define LIGHTING_STDCALL __stdcall
	#define LIGHTING_IMPORT __declspec(dllimport)
	#define LIGHTING_EXPORT __declspec(dllexport)
	*/
	#define LIGHTING_STDCALL __stdcall
	#ifdef BUILD_DLL
		#define FUNCTION __declspec(dllexport)
	#else
		#define FUNCTION __declspec(dllimport)
	#endif

	/*****************************************************************************************
		定数定義部
	******************************************************************************************/
	//#define IMAGE_NUM				(240)	/*画像数*/
	#define IMAGE_NUM				(1)		/*画像数*/
	#define MASK_NUM				(6)		/*マスク数*/
	#define STACK_NUM				(10000)	/*スタック数*/
	//#define IMAGE_WIDTH				(720)	/*画像幅*/
	//#define IMAGE_HEIGHT			(480)	/*画像高さ*/
	//#define CUTWIDTH				(256)	/*切抜画像幅(偶数)*/
	#define CUTWIDTH				(128)	/*切抜画像幅(偶数)*/
	#define CUTHEIGHT				(128)	/*切抜画像高さ(偶数)*/
	#define LIGHTING_BLACK_LEVEL	(170)	/*マスク値*/

	/*****************************************************************************************
		型定義
	******************************************************************************************/
	typedef	char LIGHTING_INT8;					/*符号付き8ビット整数*/
	typedef	unsigned char LIGHTING_UINT8;		/*符号なし8ビット整数*/
	typedef	signed short int LIGHTING_INT16;	/*符号付き16ビット整数*/
	typedef	unsigned short int LIGHTING_UINT16;	/*符号なし16ビット整数*/
	typedef	signed int LIGHTING_INT32;			/*符号付き32ビット整数*/
	typedef	unsigned int LIGHTING_UINT32;		/*符号なし32ビット整数*/
	typedef	void *LIGHTING_HANDLE;				/*ハンドル*/
	typedef LIGHTING_INT32 LIGHTING_ERROR_t;	/*エラー型*/
	typedef	LIGHTING_INT32 LIGHTING_BOOL32;		/*論理型 0:偽 それ以外：真*/

	/************************************
		2次元座標構造体。
		※LIGHTING_UINT32かLIGHTING_INT32は、雷プロジェクトに組み込むときに再度検証する。
		座標を負で使用しているかどうか。
	************************************/
	typedef struct tag_LIGHTING_POINT
	{
		LIGHTING_UINT32 x;
		LIGHTING_UINT32 y;
		LIGHTING_UINT8 val;
	}LIGHTING_POINT_t;

	/************************************
		スタック構造体。
	************************************/
	typedef struct tag_LIGHTING_TPOINT
	{
		struct
		{
			LIGHTING_POINT_t point;			/*今座標*/
			LIGHTING_POINT_t pointBefore;	/*前座標*/
			LIGHTING_UINT32	Life;			/*命*/
		}STACK[STACK_NUM];

		LIGHTING_UINT32	Flag;
	}LIGHTING_TPOINT_t;

	#if 0
		/************************************
		   線特徴構造体
		************************************/
		typedef struct tag_LIGHTING_LINE_FEATURES
		{
			struct
			{
				LIGHTING_POINT_t point;		/*座標*/
			}ImgID[ LINE_FEATURES_NUM ];
		}LIGHTING_LINE_FEATURES_t;
	#endif

	/************************************
	   マスク構造体
	************************************/
	typedef struct tag_LIGHTING_MASK_FEATURES
	{
		struct
		{
			LIGHTING_POINT_t point;		/*座標*/
		}MaskID[MASK_NUM];
	}LIGHTING_MASK_FEATURES_t;

	/************************************
	   投影データ構造体。
	************************************/
	typedef struct tag_LIGHTING_PROJ_RECONST
	{
		struct
		{
			LIGHTING_UINT8* pProjImg;	/*投影データ*/
			LIGHTING_UINT8* pConstImg;	/*逆投影データ*/
		}ImgID[CUTWIDTH];
	}LIGHTING_PROJ_RECONST_t;

	/*****************************************************************************************
		エラーコード。
		上位1バイト：大分類定義(エラー or ウォーニング)
		上位2バイト：中分類定義(モジュールエラー)
		上位3,4バイト：小分類定義(モジュール内のエラー)
		上位5,6バイト：エラー種別(代表的なエラー)
		上位7,8バイト：自由定義(モジュール内で自由に定義してよいエラー)
	******************************************************************************************/

	/* 処理成功*/
	#define LIGHTING_OK				(0x00000000)	/*処理成功*/

	/* 処理失敗*/
	/*大分類*/
	#define	LIGHTING_ERROR			(0xC0000000)	/*致命的なエラー*/
	#define	LIGHTING_WARNING		(0x80000000)	/*致命的なウォーニング*/

	/*中分類(モジュール部)*/
	#define LIGHTING_DETECTION		(0x01000000)	/*ディテクション部エラー*/
	#define LIGHTING_TRACKING		(0x02000000)	/*トラッキング部エラー*/
	#define LIGHTING_SENSOR			(0x03000000)	/*レーダ部エラー*/
	#define LIGHTING_INTEGRATION	(0x04000000)	/*統合部エラー*/

	/*小分類*/
	/*ディテクション部*/
	#define	LIGHTING_MATCHING		(0x00010000)	/*三眼マッチング*/
	#define	LIGHTING_SYN			(0x00020000)	/*Disparity合成*/
	#define	LIGHTING_UVDIS			(0x00030000)	/*U-V-disparity算出*/
	#define	LIGHTING_HOUGH			(0x00040000)	/*Hough変換*/
	#define	LIGHTING_RECOG			(0x00050000)	/*認識処理*/

	/*エラー種別*/
	#define	LIGHTING_PARAM			(0x00001000)	/*引数エラー*/
	#define	LIGHTINGS_INITIALIZE	(0x00002000)	/*初期化エラー*/
	#define	LIGHTINGS_MEMORY		(0x00003000)	/*メモリエラー*/
	#define	LIGHTING_PROCESS		(0x00004000)	/*処理エラー*/

	/*論理型*/
	#define	LIGHTING_TRUE			(1)				/*真*/
	#define	LIGHTING_FALSE			(0)				/*偽*/

	/*NULL*/
	#define LIGHTING_NULL			(0x00000000)

	/*値*/
	#define MAX_VALUE8				(255)
	#define MIN_VALUE				(0)
	#define	MAX_VALUE16				(32767)

	#ifndef PI
		#define PI					(3.14159265358979323846)  /*pi*/
	#endif

	#ifndef ABS
		#define ABS(x)				(((x)>0) ? (x) : (-(x)))
	#endif

	#ifndef MAX
		#define MAX(x,y)			(((x)>(y)) ? (x) : (y))
	#endif

	#ifndef MIN
		#define MIN(x,y)			(((x)<(y)) ? (x) : (y))
	#endif
#endif