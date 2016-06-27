/*****************************************************************************************
3-D image analysis code program
Ver 1.00/2008/06/17 �����@�V�K�쐬�B
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
		�萔��`��
	******************************************************************************************/
	//#define IMAGE_NUM				(240)	/*�摜��*/
	#define IMAGE_NUM				(1)		/*�摜��*/
	#define MASK_NUM				(6)		/*�}�X�N��*/
	#define STACK_NUM				(10000)	/*�X�^�b�N��*/
	//#define IMAGE_WIDTH				(720)	/*�摜��*/
	//#define IMAGE_HEIGHT			(480)	/*�摜����*/
	//#define CUTWIDTH				(256)	/*�ؔ��摜��(����)*/
	#define CUTWIDTH				(128)	/*�ؔ��摜��(����)*/
	#define CUTHEIGHT				(128)	/*�ؔ��摜����(����)*/
	#define LIGHTING_BLACK_LEVEL	(170)	/*�}�X�N�l*/

	/*****************************************************************************************
		�^��`
	******************************************************************************************/
	typedef	char LIGHTING_INT8;					/*�����t��8�r�b�g����*/
	typedef	unsigned char LIGHTING_UINT8;		/*�����Ȃ�8�r�b�g����*/
	typedef	signed short int LIGHTING_INT16;	/*�����t��16�r�b�g����*/
	typedef	unsigned short int LIGHTING_UINT16;	/*�����Ȃ�16�r�b�g����*/
	typedef	signed int LIGHTING_INT32;			/*�����t��32�r�b�g����*/
	typedef	unsigned int LIGHTING_UINT32;		/*�����Ȃ�32�r�b�g����*/
	typedef	void *LIGHTING_HANDLE;				/*�n���h��*/
	typedef LIGHTING_INT32 LIGHTING_ERROR_t;	/*�G���[�^*/
	typedef	LIGHTING_INT32 LIGHTING_BOOL32;		/*�_���^ 0:�U ����ȊO�F�^*/

	/************************************
		2�������W�\���́B
		��LIGHTING_UINT32��LIGHTING_INT32�́A���v���W�F�N�g�ɑg�ݍ��ނƂ��ɍēx���؂���B
		���W�𕉂Ŏg�p���Ă��邩�ǂ����B
	************************************/
	typedef struct tag_LIGHTING_POINT
	{
		LIGHTING_UINT32 x;
		LIGHTING_UINT32 y;
		LIGHTING_UINT8 val;
	}LIGHTING_POINT_t;

	/************************************
		�X�^�b�N�\���́B
	************************************/
	typedef struct tag_LIGHTING_TPOINT
	{
		struct
		{
			LIGHTING_POINT_t point;			/*�����W*/
			LIGHTING_POINT_t pointBefore;	/*�O���W*/
			LIGHTING_UINT32	Life;			/*��*/
		}STACK[STACK_NUM];

		LIGHTING_UINT32	Flag;
	}LIGHTING_TPOINT_t;

	#if 0
		/************************************
		   �������\����
		************************************/
		typedef struct tag_LIGHTING_LINE_FEATURES
		{
			struct
			{
				LIGHTING_POINT_t point;		/*���W*/
			}ImgID[ LINE_FEATURES_NUM ];
		}LIGHTING_LINE_FEATURES_t;
	#endif

	/************************************
	   �}�X�N�\����
	************************************/
	typedef struct tag_LIGHTING_MASK_FEATURES
	{
		struct
		{
			LIGHTING_POINT_t point;		/*���W*/
		}MaskID[MASK_NUM];
	}LIGHTING_MASK_FEATURES_t;

	/************************************
	   ���e�f�[�^�\���́B
	************************************/
	typedef struct tag_LIGHTING_PROJ_RECONST
	{
		struct
		{
			LIGHTING_UINT8* pProjImg;	/*���e�f�[�^*/
			LIGHTING_UINT8* pConstImg;	/*�t���e�f�[�^*/
		}ImgID[CUTWIDTH];
	}LIGHTING_PROJ_RECONST_t;

	/*****************************************************************************************
		�G���[�R�[�h�B
		���1�o�C�g�F�啪�ޒ�`(�G���[ or �E�H�[�j���O)
		���2�o�C�g�F�����ޒ�`(���W���[���G���[)
		���3,4�o�C�g�F�����ޒ�`(���W���[�����̃G���[)
		���5,6�o�C�g�F�G���[���(��\�I�ȃG���[)
		���7,8�o�C�g�F���R��`(���W���[�����Ŏ��R�ɒ�`���Ă悢�G���[)
	******************************************************************************************/

	/* ��������*/
	#define LIGHTING_OK				(0x00000000)	/*��������*/

	/* �������s*/
	/*�啪��*/
	#define	LIGHTING_ERROR			(0xC0000000)	/*�v���I�ȃG���[*/
	#define	LIGHTING_WARNING		(0x80000000)	/*�v���I�ȃE�H�[�j���O*/

	/*������(���W���[����)*/
	#define LIGHTING_DETECTION		(0x01000000)	/*�f�B�e�N�V�������G���[*/
	#define LIGHTING_TRACKING		(0x02000000)	/*�g���b�L���O���G���[*/
	#define LIGHTING_SENSOR			(0x03000000)	/*���[�_���G���[*/
	#define LIGHTING_INTEGRATION	(0x04000000)	/*�������G���[*/

	/*������*/
	/*�f�B�e�N�V������*/
	#define	LIGHTING_MATCHING		(0x00010000)	/*�O��}�b�`���O*/
	#define	LIGHTING_SYN			(0x00020000)	/*Disparity����*/
	#define	LIGHTING_UVDIS			(0x00030000)	/*U-V-disparity�Z�o*/
	#define	LIGHTING_HOUGH			(0x00040000)	/*Hough�ϊ�*/
	#define	LIGHTING_RECOG			(0x00050000)	/*�F������*/

	/*�G���[���*/
	#define	LIGHTING_PARAM			(0x00001000)	/*�����G���[*/
	#define	LIGHTINGS_INITIALIZE	(0x00002000)	/*�������G���[*/
	#define	LIGHTINGS_MEMORY		(0x00003000)	/*�������G���[*/
	#define	LIGHTING_PROCESS		(0x00004000)	/*�����G���[*/

	/*�_���^*/
	#define	LIGHTING_TRUE			(1)				/*�^*/
	#define	LIGHTING_FALSE			(0)				/*�U*/

	/*NULL*/
	#define LIGHTING_NULL			(0x00000000)

	/*�l*/
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