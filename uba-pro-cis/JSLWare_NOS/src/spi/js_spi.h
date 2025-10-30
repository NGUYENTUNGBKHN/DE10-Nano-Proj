/*==============================================================================*/
/* Copyright (C) 2012 JSL Technology. All right reserved.						*/
/* Tittle: SPI driver															*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_SPI__
#define __J_SPI__


/*==============================================================================*/
/* �o�[�W�������																*/
/*==============================================================================*/
#define	SPI_VER				100

/*==============================================================================*/
/* �n���h����`																	*/
/*==============================================================================*/
typedef struct {
	void *hdl;
} SPI_HANDLE;

/*==============================================================================*/
/* �e��f�t�@�C��(�ύX�\)														*/
/*==============================================================================*/
#define	SPIM_ACCESS_TOUT		(1000*4)	/* �^�C���A�E�g */
#define	SPIM_ACCESS_TOUT_CNT	(10000000)	/* �^�C���A�E�g�J�E���^ */

/*==============================================================================*/
/* API�����p��`																*/
/*==============================================================================*/
/* mode */
#define SPI_MODE_MASTER			0x00	/* Master Mode */
#define SPI_MODE_SLAVE			0x01	/* Slave Mode */

/* opt(Master Only) */
#define	SPIM_OPT_PACK_END		0x01	/* �o�͌�ACS��Disable�ɂ��� */
#define SPIM_OPT_COND			0x02	/* recv�f�[�^��cond�̒l�ɂȂ�܂ő҂��Ă��� */
										/* �o�b�t�@���͂��� */
#define	SPIM_8BIT_GRAN			0x00	/* 8bit�]�� */
#define	SPIM_16BIT_GRAN			0x10	/* 16bit�]�� */
#define	SPIM_32BIT_GRAN			0x20	/* 32bit�]�� */

typedef struct {
	UINT8 port;
	UINT8 mode;
} SPI_PARAM;

/* Master Only */
typedef struct {
	UINT8 opt;
	UINT8 *send;			/* NULL�w��̎��A0�f�[�^�o�� */
	UINT8 *recv;			/* NULL�w��̎��A��M�f�[�^�̓o�b�t�@�ɏo�͂��Ȃ� */
	UINT16 len;				/* ���[�h�� */
	UINT8 cond;				/* �҂����� */
} SPIM_SEND;

/*==============================================================================*/
/* API																			*/
/*==============================================================================*/
INT8 Spi_open( SPI_HANDLE *handle, SPI_PARAM *param );
void Spi_close( SPI_HANDLE *handle );
INT8 Spi_send( SPI_HANDLE *handle, UINT8 ch, SPIM_SEND *send );
INT8 Spi_init( OSW_MEM_HANDLE *mem_handle );


/*==============================================================================*/
/* �O���Ő錾����R���t�B�O���[�V�����萔										*/
/*==============================================================================*/

/* CFG_SPIM_FMT */
#define	SPIM_FMT_CLK_H_ACTIVE		0x01
#define	SPIM_FMT_CLK_L_ACTIVE		0x00
#define	SPIM_FMT_STB_H_ACTIVE		0x02
#define	SPIM_FMT_STB_L_ACTIVE		0x00

extern const UINT32 CFG_SPIM_SYSCLK[];		/* �y���t�F�����V�X�e���N���b�N���g��(Hz) */
extern const UINT16 *CFG_SPIM_STBGPIOID[];	/* STB�[�q�pGPIO_ID */
extern const UINT8 CFG_SPIM_FMT[];			/* SPI�o�̓t�H�[�}�b�g */
extern const UINT32 CFG_SPIM_CLK[];			/* SPI�ڕW�r�b�gCLK���g�� */


#endif /* __J_SPI__ */









